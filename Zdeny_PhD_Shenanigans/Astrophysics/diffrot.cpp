#include "stdafx.h"
#include "diffrot.h"
#include "Plot/Plot1D.h"
#include "Plot/Plot2D.h"

DiffrotResults calculateDiffrotProfile( const IPCsettings &ipcset, FitsTime &time, DiffrotSettings drset )
{
	int dy = drset.vFov / ( drset.ys - 1 );
	std::vector<std::vector<double>> thetas2D;
	std::vector<std::vector<double>> omegasX2D;
	std::vector<std::vector<double>> predicX2D;
	std::vector<std::vector<double>> image2D;
	std::vector<std::vector<double>> predicXs = zerovect2( 2, drset.ys );
	std::vector<double> shiftsX( drset.ys );
	std::vector<double> thetas( drset.ys );
	std::vector<double> omegasX( drset.ys );
	std::vector<double> image( drset.ys );

	std::vector<double> omegasXavg( drset.ys );
	std::vector<double> omegasXavgfit( drset.ys );

	thetas2D.reserve( drset.pics );
	omegasX2D.reserve( drset.pics );
	predicX2D.reserve( drset.pics );
	image2D.reserve( drset.pics );

	FitsImage pic1, pic2;

	for ( int pic = 0; pic < drset.pics; pic++ )
	{
		time.advanceTime( ( bool )pic * ( drset.sPic - drset.dPic ) * drset.dSec );
		loadFitsFuzzy( pic1, time );
		time.advanceTime( drset.dPic * drset.dSec );
		loadFitsFuzzy( pic2, time );

		if ( pic1.params().succload && pic2.params().succload )
		{
			double R = ( pic1.params().R + pic2.params().R ) / 2.;
			double theta0 = ( pic1.params().theta0 + pic2.params().theta0 ) / 2.;

			calculateOmegas( pic1, pic2, shiftsX, thetas, omegasX, image, predicXs, ipcset, drset, R, theta0, dy );

			if ( pic >= 10 )
			{
				double diff = mean( omegasX ) - mean( meanVertical( omegasX2D ) );
				if ( abs( diff ) > 1. )
				{
					LOG_ERROR( "Calculating diffrot profile... {}%, abnormal profile detected, diff = {}, skipping", ( double )( pic + 1 ) / drset.pics * 100, diff );
					continue;
				}
				else
				{
					LOG_SUCC( "Calculating diffrot profile... {}%, normal profile detected, diff = {}, adding", ( double )( pic + 1 ) / drset.pics * 100, diff );
				}
			}
			else
			{
				LOG_SUCC( "Calculating diffrot profile... {}%, estimating initial profile", ( double )( pic + 1 ) / drset.pics * 100 );
			}

			thetas2D.emplace_back( thetas );
			omegasX2D.emplace_back( omegasX );
			predicX2D.emplace_back( predicXs[0] );
			image2D.emplace_back( image );

			omegasXavg = meanVertical( omegasX2D );
			omegasXavgfit = polyfit( thetas, omegasXavg, 2 );

			Plot1D::plot( ( 360. / Constants::TwoPi ) * thetas, std::vector<std::vector<double>> { omegasXavg, omegasXavgfit, predicXs[0], predicXs[1], omegasX}, std::vector<std::vector<double>> {shiftsX}, "diffrot1D", "solar latitude [deg]", "horizontal plasma flow speed [deg/day]", "horizontal px shift [px]", std::vector<std::string> {"omegasXavg", "omegasXavgfit", "predicX1", "predicX2", "omegasX"}, std::vector<std::string> {"shiftsX"} );
			Plot2D::plot( applyQuantile( matFromVector( omegasX2D ), 0.01, 0.99 ), "diffrot2D", "solar latitude [deg]", "solar longitude [pics]", "horizontal plasma flow speed [deg/day]", ( 360. / Constants::TwoPi )*thetas.back(), ( 360. / Constants::TwoPi )*thetas.front(), 0, 1 );
		}
	}

	DiffrotResults dr;
	dr.SetData1D( thetas, omegasXavg, omegasXavgfit, predicXs[0], predicXs[1] );
	dr.SetData2D( image2D, omegasX2D, predicX2D );
	return dr;
}

void calculateOmegas( const FitsImage &pic1, const FitsImage &pic2, std::vector<double> &shiftsX, std::vector<double> &thetas, std::vector<double> &omegasX, std::vector<double> &image, std::vector<std::vector<double>> &predicXs, const IPCsettings &ipcset, const DiffrotSettings &drset, double R, double theta0, double dy )
{
	#pragma omp parallel for
	for ( int y = 0; y < drset.ys; y++ )
	{
		Mat crop1 = roicrop( pic1.image(), pic1.params().fitsMidX, pic1.params().fitsMidY + dy * ( double )( y - drset.ys / 2 ) + sy, ipcset.getcols(), ipcset.getrows() );
		Mat crop2 = roicrop( pic2.image(), pic2.params().fitsMidX, pic2.params().fitsMidY + dy * ( double )( y - drset.ys / 2 ) + sy, ipcset.getcols(), ipcset.getrows() );
		image[y] = pic1.image().at<ushort>( pic1.params().fitsMidX, pic1.params().fitsMidY + dy * ( double )( y - drset.ys / 2 ) + sy );
		shiftsX[y] = phasecorrel( std::move( crop1 ), std::move( crop2 ), ipcset, y == yshow ).x;
	}

	if ( drset.medianFilter )
		filterShiftsMEDIAN( shiftsX, drset.medianFilterSize );

	if ( drset.movavgFilter )
		filterShiftsMOVAVG( shiftsX, drset.movavgFilterSize );

	for ( int y = 0; y < drset.ys; y++ )
	{
		thetas[y] = asin( ( dy * ( double )( drset.ys / 2 - y ) - sy ) / R ) + theta0;
		shiftsX[y] = clamp( shiftsX[y], 0, Constants::Max );
		omegasX[y] = asin( shiftsX[y] / ( R * cos( thetas[y] ) ) ) / ( double )( drset.dPic * drset.dSec ) * ( 360. / Constants::TwoPi ) * ( 60. * 60. * 24. );
		predicXs[0][y] = predictDiffrotProfile( thetas[y], 14.713, -2.396, -1.787 );
		predicXs[1][y] = predictDiffrotProfile( thetas[y], 14.192, -1.70, -2.36 );
		// etc...
	}
}

void filterShiftsMEDIAN( std::vector<double> &shiftsX, int size )
{
	auto shiftsXma = shiftsX;
	std::vector<double> med;
	med.reserve( size );

	for ( int i = 0; i < shiftsX.size(); i++ )
	{
		med.clear();
		for ( int m = 0; m < size; m++ )
		{
			int idx = i - size / 2 + m;

			if ( idx < 0 )
				continue;
			if ( idx == shiftsX.size() )
				break;

			med.emplace_back( shiftsX[idx] );
		}

		shiftsXma[i] = median( med );
	}
	shiftsX = shiftsXma;
}

void filterShiftsMOVAVG( std::vector<double> &shiftsX, int size )
{
	auto shiftsXma = shiftsX;
	double movavg;
	int movavgcnt;

	for ( int i = 0; i < shiftsX.size(); i++ )
	{
		movavg = 0;
		movavgcnt = 0;
		for ( int m = 0; m < size; m++ )
		{
			int idx = i - size / 2 + m;

			if ( idx < 0 )
				continue;
			if ( idx == shiftsX.size() )
				break;

			movavg += shiftsX[idx];
			movavgcnt += 1;
		}
		movavg /= ( double )movavgcnt;
		shiftsXma[i] = movavg;
	}
	shiftsX = shiftsXma;
}

double predictDiffrotProfile( double theta, double A, double B, double C )
{
	return ( A + B * pow( sin( theta ), 2 ) + C * pow( sin( theta ), 4 ) );
}

void loadFitsFuzzy( FitsImage &pic, FitsTime &time )
{
	pic.reload( time.path() );

	if ( pic.params().succload )
	{
		return;
	}
	else
	{
		for ( int pm = 1; pm < plusminusbufer; pm++ )
		{
			if ( !( pm % 2 ) )
				time.advanceTime( pm );
			else
				time.advanceTime( -pm );

			pic.reload( time.path() );

			if ( pic.params().succload )
				return;
		}
	}

	time.advanceTime( plusminusbufer / 2 );
}

