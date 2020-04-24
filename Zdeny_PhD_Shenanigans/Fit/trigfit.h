#pragma once
#include "Core/functionsBaseCV.h"

inline std::vector<double> sin2sin4fit( const std::vector<double> &xdata, const std::vector<double> &ydata )
{
	int datacnt = ydata.size();
	Mat X = Mat::zeros( datacnt, 3, CV_32F ); //matice planu
	Mat Y = Mat::zeros( datacnt, 1, CV_32F ); //matice prave strany
	for ( int r = 0; r < X.rows; r++ )
	{
		Y.at<float>( r, 0 ) = ydata[r];

		X.at<float>( r, 0 ) = 1;
		X.at<float>( r, 1 ) = pow( sin( xdata[r] ), 2 );
		X.at<float>( r, 2 ) = pow( sin( xdata[r] ), 4 );
	}
	Mat coeffs = ( X.t() * X ).inv() * X.t() * Y; //least squares
	Mat fitM = X * coeffs;
	std::vector<double> fit( datacnt, 0 );
	for ( int r = 0; r < fitM.rows; r++ )
	{
		fit[r] = fitM.at<float>( r, 0 );
	}
	return fit;
}

inline std::vector<double> sin2sin4fitCoeffs( const std::vector<double> &xdata, const std::vector<double> &ydata )
{
	int datacnt = ydata.size();
	Mat X = Mat::zeros( datacnt, 3, CV_32F ); //matice planu
	Mat Y = Mat::zeros( datacnt, 1, CV_32F ); //matice prave strany
	for ( int r = 0; r < X.rows; r++ )
	{
		Y.at<float>( r, 0 ) = ydata[r];

		X.at<float>( r, 0 ) = 1;
		X.at<float>( r, 1 ) = pow( sin( xdata[r] ), 2 );
		X.at<float>( r, 2 ) = pow( sin( xdata[r] ), 4 );
	}
	Mat coeffsM = ( X.t() * X ).inv() * X.t() * Y; //least squares
	std::vector<double> coeffs( coeffsM.rows, 0 );
	for ( int r = 0; r < coeffsM.rows; r++ )
	{
		coeffs[r] = coeffsM.at<float>( r, 0 );
	}
	return coeffs;
}