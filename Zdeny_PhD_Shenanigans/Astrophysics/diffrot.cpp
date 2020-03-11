#include "stdafx.h"
#include "diffrot.h"

DiffrotResults calculateDiffrotProfile( const IPCsettings &ipcset, FitsTime &time, DiffrotSettings drset, IPlot1D *plt1, IPlot1D *plt2 )
{
	int dy = drset.vFov / ( drset.ys - 1 );

	plt1->setAxisNames( "solar latitude [deg]", "horizontal plasma flow speed [deg/day]", std::vector<std::string> {"omegasX", "omegasXavgfit", "predicX1", "predicX2"} );
	plt2->setAxisNames( "solar latitude [deg]", "horizontal px shift [px]", "theta[deg]" );

	std::vector<std::vector<double>> thetas2D;
	std::vector<std::vector<double>> omegasX2D;
	std::vector<std::vector<double>> predicXs = zerovect2( 2, drset.ys );
	std::vector<double> shiftsX( drset.ys );
	std::vector<double> thetas( drset.ys );
	std::vector<double> omegasX( drset.ys );
	std::vector<double> omegasXfit( drset.ys );
	std::vector<double> omegasXavgfit( drset.ys );

	std::vector<double> iotam( drset.ys );
	std::iota( iotam.begin(), iotam.end(), 0 );

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

			calculateOmegas( pic1, pic2, shiftsX, thetas, omegasX, predicXs, ipcset, drset, R, theta0, dy );

			omegasX2D.emplace_back( omegasX );
			thetas2D.emplace_back( thetas );

			omegasXfit = theta1Dfit( omegasX, thetas );
			omegasXavgfit = theta2Dfit( omegasX2D, thetas2D );

			drplot1( plt1, thetas, omegasX, omegasXavgfit, predicXs );
			drplot2( plt2, iotam, shiftsX, thetas );
		}
	}

	return fillDiffrotResults();
}

void calculateOmegas( const FitsImage &pic1, const FitsImage &pic2, std::vector<double> &shiftsX, std::vector<double> &thetas, std::vector<double> &omegasX,
                      std::vector<std::vector<double>> &predicXs, const IPCsettings &ipcset, const DiffrotSettings &drset, double R, double theta0, double dy )
{
	#pragma omp parallel for
	for ( int y = 0; y < drset.ys; y++ )
	{
		Mat crop1 = roicrop( pic1.image(), pic1.params().fitsMidX, pic1.params().fitsMidY + dy * ( double )( y - drset.ys / 2 ) + sy, ipcset.getcols(), ipcset.getrows() );
		Mat crop2 = roicrop( pic2.image(), pic2.params().fitsMidX, pic2.params().fitsMidY + dy * ( double )( y - drset.ys / 2 ) + sy, ipcset.getcols(), ipcset.getrows() );
		shiftsX[y] = phasecorrel( std::move( crop1 ), std::move( crop2 ), ipcset, nullptr, y == yshow ).x;
	}

	if ( drset.filter )
	{
		filterShiftsMEDIAN( shiftsX );
		filterShiftsMOVAVG( shiftsX );
	}

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

std::vector<double> theta1Dfit( const std::vector<double> &omegas, const std::vector<double> &thetas )
{
	return polyfit( thetas, omegas, 2 );
}

std::vector<double> theta2Dfit( const std::vector<std::vector<double>> &omegasX2D, const std::vector<std::vector<double>> &thetas2D )
{
	int I = omegasX2D.size();
	int J = omegasX2D[0].size();

	std::vector<double> omegasAll( I * J );
	std::vector<double> thetasAll( I * J );

	for ( int i = 0; i < I; i++ )
	{
		for ( int j = 0; j < J; j++ )
		{
			omegasAll[i * J + j] = omegasX2D[i][j];
			thetasAll[i * J + j] = thetas2D[i][j];
		}
	}

	return polyfit( thetasAll, omegasAll, 2 );
}

void drplot1( IPlot1D *plt1, const std::vector<double> &thetas, const std::vector<double> &omegasX, const std::vector<double> &omegasXavgfit,
              const std::vector<std::vector<double>> &predicXs )
{
	plt1->plot( ( 360. / Constants::TwoPi ) * thetas, std::vector<std::vector<double>> {omegasX, omegasXavgfit, predicXs[0], predicXs[1]} );
}

void drplot2( IPlot1D *plt2, const std::vector<double> &iotam, const std::vector<double> &shiftsX, const std::vector<double> &thetas )
{
	plt2->plot( iotam, shiftsX, ( 360. / Constants::TwoPi ) * thetas );
}

void filterShiftsMOVAVG( std::vector<double> &shiftsX )
{
	auto shiftsXma = shiftsX;
	double movavg;

	for ( int i = 0; i < shiftsX.size(); i++ )
	{
		movavg = 0;
		for ( int m = 0; m < movavgWindow; m++ )
		{
			int idx = i - movavgWindow / 2 + m;

			if ( idx < 0 || idx == shiftsX.size() )
				break;

			movavg += shiftsX[idx];
		}
		movavg /= ( double )movavgWindow;
		shiftsXma[i] = movavg;
	}
	shiftsX = shiftsXma;
}

void filterShiftsMEDIAN( std::vector<double> &shiftsX )
{
	auto shiftsXma = shiftsX;
	std::vector<double> med( medianWindow );

	for ( int i = 0; i < shiftsX.size(); i++ )
	{
		for ( int m = 0; m < movavgWindow; m++ )
		{
			int idx = i - movavgWindow / 2 + m;
			med[m] = shiftsX[idx];
		}

		shiftsXma[i] = median( med );
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

DiffrotResults fillDiffrotResults()
{
	return DiffrotResults();
}