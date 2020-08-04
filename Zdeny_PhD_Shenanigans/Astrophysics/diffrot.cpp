#include "stdafx.h"
#include "diffrot.h"
#include "Plot/Plot1D.h"
#include "Plot/Plot2D.h"

DiffrotResults calculateDiffrotProfile( const IPCsettings &ipcset, FitsTime &time, const DiffrotSettings &drset )
{
	int dy = drset.vFov / ( drset.ys - 1 );
	std::vector<std::vector<double>> thetas2D;
	std::vector<std::vector<double>> omegasX2D;
	std::vector<std::vector<double>> omegasY2D;
	std::vector<std::vector<double>> shiftsX2D;
	std::vector<std::vector<double>> shiftsY2D;
	std::vector<std::vector<double>> image2D;
	std::vector<std::vector<double>> predicXs = zerovect2( 2, drset.ys );
	std::vector<double> thetas( drset.ys );
	std::vector<double> shiftsX( drset.ys );
	std::vector<double> shiftsY( drset.ys );
	std::vector<double> omegasX( drset.ys );
	std::vector<double> omegasY( drset.ys );
	std::vector<double> image( drset.ys );

	std::vector<double> thetasavg( drset.ys );
	std::vector<double> omegasXavg( drset.ys );
	std::vector<double> omegasYavg( drset.ys );
	std::vector<double> shiftsXavg( drset.ys );
	std::vector<double> shiftsYavg( drset.ys );
	std::vector<double> omegasXavgpolyfit( drset.ys );
	std::vector<double> omegasYavgpolyfit( drset.ys );

	thetas2D.reserve( drset.pics );
	omegasX2D.reserve( drset.pics );
	omegasY2D.reserve( drset.pics );
	shiftsX2D.reserve( drset.pics );
	shiftsY2D.reserve( drset.pics );
	image2D.reserve( drset.pics );

	FitsImage pic1, pic2;
	int lag1, lag2;

	for ( int pic = 0; pic < drset.pics; pic++ )
	{
		time.advanceTime( ( bool )pic * ( drset.sPic - drset.dPic ) * drset.dSec );
		loadFitsFuzzy( pic1, time, lag1 );
		time.advanceTime( drset.dPic * drset.dSec );
		loadFitsFuzzy( pic2, time, lag2 );

		if ( pic1.params().succload && pic2.params().succload )
		{
			double R = ( pic1.params().R + pic2.params().R ) / 2.;
			double theta0 = ( pic1.params().theta0 + pic2.params().theta0 ) / 2.;

			calculateOmegas( pic1, pic2, shiftsX, shiftsY, thetas, omegasX, omegasY, image, predicXs, ipcset, drset, R, theta0, dy, lag1, lag2 );

			image2D.emplace_back( image );

			if ( pic < 10 )
			{
				LOG_SUCC( "{} / {} estimating initial profile", pic + 1, drset.pics );
				thetas2D.emplace_back( thetas );
				omegasX2D.emplace_back( omegasX );
				omegasY2D.emplace_back( omegasY );
				shiftsX2D.emplace_back( shiftsX );
				shiftsY2D.emplace_back( shiftsY );
			}
			else
			{
				double diffX = mean( omegasX ) - mean( omegasXavg );
				double diffY = mean( omegasY ) - mean( omegasYavg );

				const double diffThreshX = 1;//1
				const double diffThreshY = 2;//2

				// filter outlier X data
				if ( abs( diffX ) < diffThreshX )
				{
					thetas2D.emplace_back( thetas );
					omegasX2D.emplace_back( omegasX );
					shiftsX2D.emplace_back( shiftsX );
				}
				else
				{
					LOG_ERROR( "Abnormal profile detected, diff X = {}, skipping", diffX );
				}

				// filter outlier Y data
				if ( abs( diffY ) < diffThreshY )
				{
					omegasY2D.emplace_back( omegasY );
					shiftsY2D.emplace_back( shiftsY );
				}
				else
				{
					LOG_ERROR( "Abnormal profile detected, diff Y = {}, skipping", diffY );
				}

				// log progress
				if ( abs( diffX ) < diffThreshX && abs( diffY ) < diffThreshY )
				{
					LOG_SUCC( "{} / {} ... diff X/Y = {}/{}, adding", pic + 1, drset.pics, diffX, diffY );
				}
			}

			thetasavg = meanVertical( thetas2D );
			omegasXavg = meanVertical( omegasX2D );
			omegasYavg = meanVertical( omegasY2D );

			if ( drset.visual )
			{
				shiftsXavg = meanVertical( shiftsX2D );
				shiftsYavg = meanVertical( shiftsY2D );
				omegasXavgpolyfit = polyfit( thetasavg, omegasXavg, 2 );
				omegasYavgpolyfit = polyfit( thetasavg, omegasYavg, 3 );

				Plot1D::plot( ( 360. / Constants::TwoPi ) * thetasavg, std::vector<std::vector<double>> { omegasXavg, omegasXavgpolyfit, predicXs[0], predicXs[1], omegasX}, std::vector<std::vector<double>> {shiftsXavg}, "diffrot1DX", "solar latitude [deg]", "horizontal material flow speed [deg/day]", "horizontal px shift [px]", std::vector<std::string> {"omegasXavg", "omegasXavgpolyfit", "predic1", "predic2", "omegasX"}, std::vector<std::string> {"shiftsXavg"} );
				Plot1D::plot( ( 360. / Constants::TwoPi ) * thetasavg, std::vector<std::vector<double>> { omegasYavg, omegasYavgpolyfit, omegasY}, std::vector<std::vector<double>> {shiftsYavg}, "diffrot1DY", "solar latitude [deg]", "vertical material flow speed [deg/day]", "vertical px shift [px]", std::vector<std::string> {"omegasYavg", "omegasYavgpolyfit", "omegasY"}, std::vector<std::string> {"shiftsYavg"} );

				Plot2D::plot( applyQuantile( matFromVector( omegasX2D ), 0.01, 0.99 ), "diffrot2DX", "solar latitude [deg]", "solar longitude [pics]", "horizontal material flow speed [deg/day]", ( 360. / Constants::TwoPi )*thetasavg.back(), ( 360. / Constants::TwoPi )*thetasavg.front(), 0, 1, 2 );
				Plot2D::plot( applyQuantile( matFromVector( omegasY2D ), 0.01, 0.99 ), "diffrot2DY", "solar latitude [deg]", "solar longitude [pics]", "vertical material flow speed [deg/day]", ( 360. / Constants::TwoPi )*thetasavg.back(), ( 360. / Constants::TwoPi )*thetasavg.front(), 0, 1, 2 );
			}
		}
	}

	thetasavg = meanVertical( thetas2D );
	omegasXavg = meanVertical( omegasX2D );
	omegasYavg = meanVertical( omegasY2D );
	shiftsXavg = meanVertical( shiftsX2D );
	shiftsYavg = meanVertical( shiftsY2D );

	DiffrotResults dr;
	dr.SetData1D( thetasavg, omegasXavg, omegasYavg, shiftsXavg, shiftsYavg );
	dr.SetData2D( image2D, omegasX2D, omegasY2D, shiftsX2D, shiftsY2D );
	dr.SetParams( drset.pics, drset.sPic, drset.savepath );

	return dr;
}

void calculateOmegas( const FitsImage &pic1, const FitsImage &pic2, std::vector<double> &shiftsX, std::vector<double> &shiftsY, std::vector<double> &thetas, std::vector<double> &omegasX, std::vector<double> &omegasY, std::vector<double> &image, std::vector<std::vector<double>> &predicXs, const IPCsettings &ipcset, const DiffrotSettings &drset, double R, double theta0, double dy, int lag1, int lag2 )
{
	#pragma omp parallel for
	for ( int y = 0; y < drset.ys; y++ )
	{
		Mat crop1 = roicrop( pic1.image(), pic1.params().fitsMidX, pic1.params().fitsMidY + dy * ( double )( y - drset.ys / 2 ) + sy, ipcset.getcols(), ipcset.getrows() );
		Mat crop2 = roicrop( pic2.image(), pic2.params().fitsMidX, pic2.params().fitsMidY + dy * ( double )( y - drset.ys / 2 ) + sy, ipcset.getcols(), ipcset.getrows() );
		image[y] = pic1.image().at<ushort>( pic1.params().fitsMidX, pic1.params().fitsMidY + dy * ( double )( y - drset.ys / 2 ) + sy );
		auto shift = phasecorrel( std::move( crop1 ), std::move( crop2 ), ipcset, y == yshow );
		shiftsX[y] = shift.x;
		shiftsY[y] = shift.y;
	}

	if ( drset.medianFilter )
	{
		filterMedian( shiftsX, drset.medianFilterSize );
		filterMedian( shiftsY, drset.medianFilterSize );
	}

	if ( drset.movavgFilter )
	{
		filterMovavg( shiftsX, drset.movavgFilterSize );
		filterMovavg( shiftsY, drset.movavgFilterSize );
	}

	for ( int y = 0; y < drset.ys; y++ )
	{
		thetas[y] = asin( ( dy * ( double )( drset.ys / 2 - y ) - sy ) / R ) + theta0;
		shiftsX[y] = clamp( shiftsX[y], 0, Constants::Max );
		shiftsY[y] = clamp( shiftsY[y], Constants::Min, Constants::Max );
		omegasX[y] = asin( shiftsX[y] / ( R * cos( thetas[y] ) ) ) / ( double )( drset.dPic * drset.dSec + lag2 - lag1 ) * ( 360. / Constants::TwoPi ) * ( 60. * 60. * 24. );
		omegasY[y] = ( thetas[y] - asin( sin( thetas[y] ) - shiftsY[y] / R ) ) / ( double )( drset.dPic * drset.dSec + lag2 - lag1 ) * ( 360. / Constants::TwoPi ) * ( 60. * 60. * 24. );
		predicXs[0][y] = predictDiffrotProfile( thetas[y], 14.296, -1.847, -2.615 ); //Derek A. Lamb (2017)
		predicXs[1][y] = predictDiffrotProfile( thetas[y], 14.192, -1.70, -2.36 ); //Howard et al. (1983)
		// etc...
	}
}

void loadFitsFuzzy( FitsImage &pic, FitsTime &time, int &lag )
{
	pic.reload( time.path() );

	if ( pic.params().succload )
	{
		lag = 0;
		return;
	}
	else
	{
		for ( int pm = 1; pm < plusminusbufer; pm++ )
		{
			int shift = 0;;

			if ( !( pm % 2 ) )
			{
				time.advanceTime( pm );
				shift += pm;
			}
			else
			{
				time.advanceTime( -pm );
				shift += -pm;
			}

			pic.reload( time.path() );
			if ( pic.params().succload )
			{
				lag = shift;
				return;
			}
		}
	}
	lag = 0;
	time.advanceTime( plusminusbufer / 2 );
}
