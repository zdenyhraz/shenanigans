#include "stdafx.h"
#include "diffrot.h"

DiffrotResults calculateDiffrotProfile( const IPCsettings &ipcset, FitsTime &time, DiffrotSettings drset, IPlot1D *plt1, IPlot1D *plt2 )
{
	int dy = drset.vFov / ( drset.ys - 1 );

	plt1->setAxisNames( "solar latitude [deg]", "horizontal plasma flow speed [deg/day]", std::vector<std::string> {"omegasX", "omegasXavg", "omegasXavgfit", "predicX1", "predicX2"} );
	plt2->setAxisNames( "solar latitude [deg]", "horizontal px shift [px]", "theta[deg]" );

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

	std::vector<double> iotam( drset.ys );
	std::iota( iotam.begin(), iotam.end(), 0 );

	thetas2D.reserve( drset.pics );
	omegasX2D.reserve( drset.pics );
	predicX2D.reserve( drset.pics );
	image2D.reserve( drset.pics );

	FitsImage pic1, pic2;

	for ( int pic = 0; pic < drset.pics; pic++ )
	{
		LOG_INFO( "Calculating differential rotation profile... {}%", ( double )( pic + 1 ) / drset.pics * 100 );

		time.advanceTime( ( bool )pic * ( drset.sPic - drset.dPic ) * drset.dSec );
		loadFitsFuzzy( pic1, time );
		time.advanceTime( drset.dPic * drset.dSec );
		loadFitsFuzzy( pic2, time );

		if ( pic1.params().succload && pic2.params().succload )
		{
			double R = ( pic1.params().R + pic2.params().R ) / 2.;
			double theta0 = ( pic1.params().theta0 + pic2.params().theta0 ) / 2.;

			calculateOmegas( pic1, pic2, shiftsX, thetas, omegasX, image, predicXs, ipcset, drset, R, theta0, dy );

			if ( pic > 5 )
			{
				double diff = mean( omegasX ) - mean( meanVertical( omegasX2D ) );
				if ( abs( diff ) > 2. )
				{
					LOG_ERROR( "Outlier profile detected, diff = {}, skipping", diff );
					continue;
				}
				else
				{
					LOG_DEBUG( "Normal profile detected, diff = {}, adding", diff );
				}
			}

			thetas2D.emplace_back( thetas );
			omegasX2D.emplace_back( omegasX );
			predicX2D.emplace_back( predicXs[0] );
			image2D.emplace_back( image );

			omegasXavg = meanVertical( omegasX2D );
			omegasXavgfit = thetaFit( omegasXavg, thetas );

			drplot1( plt1, thetas, omegasX, omegasXavg, omegasXavgfit, predicXs );
			drplot2( plt2, iotam, shiftsX, thetas );
		}
	}

	DiffrotResults dr;
	dr.SetData( matFromVector( image2D, true ), matFromVector( omegasX2D, true ), matFromVector( predicX2D, true ) );
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
		shiftsX[y] = phasecorrel( std::move( crop1 ), std::move( crop2 ), ipcset, nullptr, y == yshow ).x;
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

void drplot1( IPlot1D *plt1, const std::vector<double> &thetas, const std::vector<double> &omegasX, const std::vector<double> &omegasXavg, const std::vector<double> &omegasXavgfit, const std::vector<std::vector<double>> &predicXs )
{
	plt1->plot( ( 360. / Constants::TwoPi ) * thetas, std::vector<std::vector<double>> {omegasX, omegasXavg, omegasXavgfit, predicXs[0], predicXs[1]} );
}

void drplot2( IPlot1D *plt2, const std::vector<double> &iotam, const std::vector<double> &shiftsX, const std::vector<double> &thetas )
{
	plt2->plot( iotam, shiftsX, ( 360. / Constants::TwoPi ) * thetas );
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

std::vector<double> thetaFit( const std::vector<double> &omegas, const std::vector<double> &thetas )
{
	return polyfit( thetas, omegas, 2 );
}
