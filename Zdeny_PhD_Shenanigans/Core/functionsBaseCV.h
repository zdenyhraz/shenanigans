#pragma once
#include "Core/functionsBaseSTL.h"
#include "opencv2/opencv.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

enum ColorMapStyle { CM_JET, CM_BINARY };
enum CombinePicsStyle { huebright, bluered };

inline std::pair<double, double> minMaxMat( const Mat &sourceimg )
{
	double minR, maxR;
	minMaxLoc( sourceimg, &minR, &maxR, nullptr, nullptr );
	return make_pair( minR, maxR );
}

inline std::vector<double> polyfitcore1d( const std::vector<double> &xdata, const std::vector<double> &ydata, int degree )
{
	int datacnt = ydata.size();
	Mat X = Mat::zeros( datacnt, degree + 1, CV_32F ); //matice planu
	Mat Y = Mat::zeros( datacnt, 1, CV_32F ); //matice prave strany
	for ( int r = 0; r < X.rows; r++ )
	{
		Y.at<float>( r, 0 ) = ydata[r];
		for ( int c = 0; c < X.cols; c++ )
		{
			X.at<float>( r, c ) = pow( xdata[r], c );
		}
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

inline std::vector<double> polyfit( const std::vector<double> &ydata, int degree )
{
	std::vector<double> xdata( ydata.size() );
	std::iota( xdata.begin(), xdata.end(), 1 );
	return polyfitcore1d( xdata, ydata, degree );
}

inline std::vector<double> polyfit( const std::vector<double> &xdata, const std::vector<double> &ydata, int degree )
{
	return polyfitcore1d( xdata, ydata, degree );
}

inline Mat polyfitcore2d( const std::vector<double> &xdata, const std::vector<double> &ydata, const std::vector<double> &zdata, int degree, double xmin, double xmax, double ymin, double ymax, double xcnt, double ycnt )
{
	int datacnt = ydata.size();
	Mat X = Mat::zeros( datacnt, 2 * degree + 1, CV_32F ); //matice planu
	Mat Y = Mat::zeros( datacnt, 1, CV_32F ); //matice prave strany
	for ( int r = 0; r < X.rows; r++ )
	{
		Y.at<float>( r, 0 ) = ydata[r];
		for ( int c = 0; c < X.cols; c++ )
		{
			if ( !c )
				X.at<float>( r, c ) = 1;
			else
			{
				if ( c % 2 )
					X.at<float>( r, c ) = pow( xdata[r], ceil( ( float )c / 2 ) );
				else
					X.at<float>( r, c ) = pow( ydata[r], ceil( ( float )c / 2 ) );
			}
		}
	}
	Mat coeffs = ( X.t() * X ).inv() * X.t() * Y; //least squares

	Mat fit = Mat::zeros( ycnt, xcnt, CV_32F );
	for ( int r = 0; r < fit.rows; r++ )
	{
		for ( int c = 0; c < fit.cols; c++ )
		{
			double x = xmin + ( ( float )c / ( fit.cols - 1 ) ) * ( xmax - xmin );
			double y = ymin + ( ( float )r / ( fit.rows - 1 ) ) * ( ymax - ymin );
			Mat X1 = Mat::zeros( 1, 2 * degree + 1, CV_32F ); //matice planu pro 1 point (x,y)
			for ( int c1 = 0; c1 < X1.cols; c1++ )
			{
				if ( !c1 )
					X1.at<float>( 0, c1 ) = 1;
				else
				{
					if ( c1 % 2 )
						X1.at<float>( 0, c1 ) = pow( x, ceil( ( float )c1 / 2 ) );
					else
						X1.at<float>( 0, c1 ) = pow( y, ceil( ( float )c1 / 2 ) );
				}
			}

			Mat Z = X1 * coeffs;
			fit.at<float>( r, c ) = Z.at<float>( 0, 0 );
		}
	}
	return fit;
}

inline Mat polyfit( const std::vector<double> &xdata, const std::vector<double> &ydata, const std::vector<double> &zdata, int degree, double xmin, double xmax, double ymin, double ymax, double xcnt, double ycnt )
{
	return polyfitcore2d( xdata, ydata, zdata, degree, xmin, xmax, ymin, ymax, xcnt, ycnt );
}

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

inline std::vector<double> polyfitCoeffs( const std::vector<double> &xdata, const std::vector<double> &ydata, int degree )
{
	int datacnt = ydata.size();
	Mat X = Mat::zeros( datacnt, degree + 1, CV_32F ); //matice planu
	Mat Y = Mat::zeros( datacnt, 1, CV_32F ); //matice prave strany
	for ( int r = 0; r < X.rows; r++ )
	{
		Y.at<float>( r, 0 ) = ydata[r];
		for ( int c = 0; c < X.cols; c++ )
		{
			X.at<float>( r, c ) = pow( xdata[r], c );
		}
	}
	Mat coeffsM = ( X.t() * X ).inv() * X.t() * Y; //least squares
	std::vector<double> coeffs( coeffsM.rows, 0 );
	for ( int r = 0; r < coeffsM.rows; r++ )
	{
		coeffs[r] = coeffsM.at<float>( r, 0 );
	}
	return coeffs;
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

inline Mat crosshair( const Mat &sourceimgIn, cv::Point point )
{
	Mat sourceimg = sourceimgIn.clone();
	Scalar CrosshairColor = Scalar( 0 );
	int thickness = max( sourceimg.cols / 150, 1 );
	int inner = sourceimg.cols / 50;
	int outer = sourceimg.cols / 18;
	cv::Point offset1( 0, outer );
	cv::Point offset2( outer, 0 );
	cv::Point offset3( 0, inner );
	cv::Point offset4( inner, 0 );
	normalize( sourceimg, sourceimg, 0, 255, NORM_MINMAX );
	sourceimg.convertTo( sourceimg, CV_8U );

	circle( sourceimg, point, inner, CrosshairColor, thickness, 0, 0 );
	line( sourceimg, point + offset3, point + offset1, CrosshairColor, thickness );
	line( sourceimg, point + offset4, point + offset2, CrosshairColor, thickness );
	line( sourceimg, point - offset3, point - offset1, CrosshairColor, thickness );
	line( sourceimg, point - offset4, point - offset2, CrosshairColor, thickness );
	return sourceimg;
}

inline Mat xko( const Mat &sourceimgIn, cv::Point point, Scalar CrosshairColor, std::string inputType )
{
	Mat sourceimg = sourceimgIn.clone();
	int inner = 0;
	int outer = 4;
	cv::Point offset1( -outer, -outer );
	cv::Point offset2( outer, -outer );
	cv::Point offset3( outer, outer );
	cv::Point offset4( -outer, outer );
	Mat sourceimgCLR = Mat::zeros( sourceimg.size(), CV_16UC3 );
	if ( inputType == "CLR" )
	{
		normalize( sourceimg, sourceimg, 0, 65535, NORM_MINMAX );
		sourceimg.convertTo( sourceimg, CV_16UC3 );
		sourceimg.copyTo( sourceimgCLR );
	}
	if ( inputType == "GRS" )
	{
		normalize( sourceimg, sourceimg, 0, 65535, NORM_MINMAX );
		sourceimg.convertTo( sourceimg, CV_16UC1 );
		for ( int r = 0; r < sourceimg.rows; r++ )
		{
			for ( int c = 0; c < sourceimg.cols; c++ )
			{
				sourceimgCLR.at<Vec3w>( r, c )[0] = sourceimg.at<ushort>( r, c );
				sourceimgCLR.at<Vec3w>( r, c )[1] = sourceimg.at<ushort>( r, c );
				sourceimgCLR.at<Vec3w>( r, c )[2] = sourceimg.at<ushort>( r, c );
			}
		}
	}
	double THICCness = 1;
	//circle(sourceimgCLR, point, inner, CrosshairColor, 2);
	circle( sourceimgCLR, point, 5, CrosshairColor, THICCness, 0, 0 );
	line( sourceimgCLR, point + offset3, point + offset1, CrosshairColor, THICCness );
	line( sourceimgCLR, point + offset4, point + offset2, CrosshairColor, THICCness );
	line( sourceimgCLR, point - offset3, point - offset1, CrosshairColor, THICCness );
	line( sourceimgCLR, point - offset4, point - offset2, CrosshairColor, THICCness );
	return sourceimgCLR;
}

inline Mat pointik( const Mat &sourceimgIn, cv::Point point, Scalar CrosshairColor, std::string inputType )
{
	Mat sourceimg = sourceimgIn.clone();
	int inner = 1;
	int outer = 0;
	cv::Point offset1( -outer, -outer );
	cv::Point offset2( outer, -outer );
	cv::Point offset3( outer, outer );
	cv::Point offset4( -outer, outer );
	Mat sourceimgCLR = Mat::zeros( sourceimg.size(), CV_16UC3 );
	if ( inputType == "CLR" )
	{
		normalize( sourceimg, sourceimg, 0, 65535, NORM_MINMAX );
		sourceimg.convertTo( sourceimg, CV_16UC1 );
		sourceimg.copyTo( sourceimgCLR );
	}
	if ( inputType == "GRS" )
	{
		normalize( sourceimg, sourceimg, 0, 65535, NORM_MINMAX );
		sourceimg.convertTo( sourceimg, CV_16UC1 );
		for ( int r = 0; r < sourceimg.rows; r++ )
		{
			for ( int c = 0; c < sourceimg.cols; c++ )
			{
				sourceimgCLR.at<Vec3w>( r, c )[0] = sourceimg.at<ushort>( r, c );
				sourceimgCLR.at<Vec3w>( r, c )[1] = sourceimg.at<ushort>( r, c );
				sourceimgCLR.at<Vec3w>( r, c )[2] = sourceimg.at<ushort>( r, c );
			}
		}
	}
	circle( sourceimgCLR, point, inner, CrosshairColor, 2 );
	return sourceimgCLR;
}

inline Mat roicrop( const Mat &sourceimgIn, int x, int y, int w, int h )
{
	Rect roi = Rect( x - std::floor( ( double )w / 2. ), y - std::floor( ( double )h / 2. ), w, h );
	Mat crop = sourceimgIn( roi );
	return crop.clone();
}

inline Mat kirkl( unsigned size )
{
	Mat kirkl = Mat::zeros( size, size, CV_32F );
	for ( int r = 0; r < size; r++ )
	{
		for ( int c = 0; c < size; c++ )
		{
			if ( ( sqr( ( double )r - floor( size / 2 ) ) + sqr( ( double )c - floor( size / 2 ) ) ) < sqr( ( double )size / 2 + 1 ) )
			{
				kirkl.at<float>( r, c ) = 1.;
			}
			else
			{
				kirkl.at<float>( r, c ) = 0.;
			}
		}
	}
	return kirkl;
}

inline Mat kirkl( int rows, int cols, unsigned radius )
{
	Mat kirkl = Mat::zeros( rows, cols, CV_32F );
	for ( int r = 0; r < rows; r++ )
	{
		for ( int c = 0; c < cols; c++ )
		{
			if ( ( sqr( ( double )r - floor( rows / 2 ) ) + sqr( ( double )c - floor( cols / 2 ) ) ) < sqr( radius ) )
			{
				kirkl.at<float>( r, c ) = 1.;
			}
			else
			{
				kirkl.at<float>( r, c ) = 0.;
			}
		}
	}
	return kirkl;
}

inline Mat kirklcrop( const Mat &sourceimgIn, int x, int y, int diameter )
{
	Mat crop = roicrop( sourceimgIn, x, y, diameter, diameter );
	Mat kirklik = kirkl( diameter );
	return crop.mul( kirklik );
}

inline Point2f findCentroidDouble( const Mat &sourceimg )
{
	double M = 0.0;
	double My = 0.0;
	double Mx = 0.0;
	for ( int r = 0; r < sourceimg.rows; r++ )
	{
		for ( int c = 0; c < sourceimg.cols; c++ )
		{
			M += sourceimg.at<float>( r, c );
			My += ( double )r * sourceimg.at<float>( r, c );
			Mx += ( double )c * sourceimg.at<float>( r, c );
		}
	}

	Point2f ret( Mx / M, My / M );

	if ( ret.x < 0 || ret.y < 0 || ret.x > sourceimg.cols || ret.y > sourceimg.rows )
		return Point2f( sourceimg.cols / 2, sourceimg.rows / 2 );
	else
		return ret;
}

inline std::tuple<float, float, float> colorMapJET( float x, float caxisMin = 0, float caxisMax = 1 )
{
	if ( x == 0 )
		return std::make_tuple( 0., 0., 0. );

	float val = 1;
	float B, G, R;
	float sh = 0.125 * ( caxisMax - caxisMin );
	float start = caxisMin;
	float mid = caxisMin + 0.5 * ( caxisMax - caxisMin );
	float end = caxisMax;

	B = ( x > ( start + sh ) ) ? clamp( -val / 2 / sh * x + val / 2 / sh * ( mid + sh ), 0, val ) : ( x < start ? val / 2 : clamp( val / 2 / sh * x + val / 2 - val / 2 / sh * start, 0, val ) );
	G = ( x < mid ) ? clamp( val / 2 / sh * x - val / 2 / sh * ( start + sh ), 0, val ) : clamp( -val / 2 / sh * x + val / 2 / sh * ( end - sh ), 0, val );
	R = ( x < ( end - sh ) ) ? clamp( val / 2 / sh * x - val / 2 / sh * ( mid - sh ), 0, val ) : ( x > end ? val / 2 : clamp( -val / 2 / sh * x + val / 2 + val / 2 / sh * end, 0, val ) );

	return std::make_tuple( B, G, R );
}

inline Scalar colorMapJet( float x, float caxisMin = 0, float caxisMax = 1 )
{
	float val = 255;
	float B, G, R;
	float sh = 0.125 * ( caxisMax - caxisMin );
	float start = caxisMin;
	float mid = caxisMin + 0.5 * ( caxisMax - caxisMin );
	float end = caxisMax;

	B = ( x > ( start + sh ) ) ? clamp( -val / 2 / sh * x + val / 2 / sh * ( mid + sh ), 0, val ) : ( x < start ? val / 2 : clamp( val / 2 / sh * x + val / 2 - val / 2 / sh * start, 0, val ) );
	G = ( x < mid ) ? clamp( val / 2 / sh * x - val / 2 / sh * ( start + sh ), 0, val ) : clamp( -val / 2 / sh * x + val / 2 / sh * ( end - sh ), 0, val );
	R = ( x < ( end - sh ) ) ? clamp( val / 2 / sh * x - val / 2 / sh * ( mid - sh ), 0, val ) : ( x > end ? val / 2 : clamp( -val / 2 / sh * x + val / 2 + val / 2 / sh * end, 0, val ) );

	return Scalar( B, G, R );
}

inline std::tuple<int, int, int> colorMapBINARY( double x, double caxisMin = -1, double caxisMax = 1, double sigma = 1 )
{
	double B, G, R;

	if ( sigma == 0 ) //linear
	{
		if ( x < 0 )
		{
			B = 255;
			G = 255. / -caxisMin * x + 255;
			R = 255. / -caxisMin * x + 255;
		}
		else
		{
			B = 255 - 255. / caxisMax * x;
			G = 255 - 255. / caxisMax * x;
			R = 255;
		}
	}
	else//gaussian
	{
		double delta = caxisMax - caxisMin;
		if ( x < 0 )
		{
			B = 255;
			G = gaussian1D( x, 255, 0, sigma * delta / 10 );
			R = gaussian1D( x, 255, 0, sigma * delta / 10 );
		}
		else
		{
			B = gaussian1D( x, 255, 0, sigma * delta / 10 );
			G = gaussian1D( x, 255, 0, sigma * delta / 10 );
			R = 255;
		}
	}

	return std::make_tuple( B, G, R );
}

inline std::tuple<double, double, double> BGR_to_HUE( std::tuple<int, int, int> BGR )
{
	double ratioBG = ( double )std::get<0>( BGR ) / std::get<1>( BGR );
	double ratioGR = ( double )std::get<1>( BGR ) / std::get<2>( BGR );
	double ratioRB = ( double )std::get<2>( BGR ) / std::get<0>( BGR );
	return std::make_tuple( ratioBG, ratioGR, ratioRB );
}

inline std::tuple<int, int, int> HUE_to_BGR( std::tuple<double, double, double> HUE, double brightness, std::tuple<int, int, int> BGRsource )
{
	int B = 1, G = 1, R = 1;

	if ( std::get<0>( HUE ) == 0 ) B = 0; //B is 0
	if ( std::get<1>( HUE ) == 0 ) G = 0; //G is 0
	if ( std::get<2>( HUE ) == 0 ) R = 0; //R is 0

	if ( isnan( std::get<0>( HUE ) ) ) //R idk
	{
		B = 0;
		G = 0;
		R = brightness / 255 * std::get<2>( BGRsource );
	}
	else if ( isnan( std::get<1>( HUE ) ) ) //B idk
	{
		G = 0;
		R = 0;
		B = brightness / 255 * std::get<0>( BGRsource );
	}
	else if ( isnan( std::get<2>( HUE ) ) ) //G idk
	{
		R = 0;
		B = 0;
		G = brightness / 255 * std::get<1>( BGRsource );
	}
	else if ( std::get<0>( HUE ) > 1 ) //max is B or R
	{
		if ( std::get<1>( HUE ) > 1 ) //B is max
		{
			if ( B ) B = brightness;
			if ( R ) R = std::get<2>( HUE ) * B;
			if ( G ) G = B / std::get<0>( HUE );
		}
		else//R is max
		{
			if ( R ) R = brightness;
			if ( G ) G = std::get<1>( HUE ) * R;
			if ( B ) B = R / std::get<2>( HUE );
		}
	}
	else if ( std::get<0>( HUE ) <= 1 ) //max is G or R
	{
		if ( std::get<1>( HUE ) > 1 ) //G is max
		{
			if ( G ) G = brightness;
			if ( R ) R = G / std::get<1>( HUE );
			if ( B ) B = std::get<0>( HUE ) * G;
		}
		else//R is max
		{
			if ( R ) R = brightness;
			if ( G ) G = std::get<1>( HUE ) * R;
			if ( B ) B = R / std::get<2>( HUE );
		}
	}

	return std::make_tuple( B, G, R );
}

inline Mat combineTwoPics( const Mat &source1In, const Mat &source2In, CombinePicsStyle style, double sigma = 1 )
{
	Mat source1 = source1In.clone();
	Mat source2 = source2In.clone();
	Mat result = Mat::zeros( source1.rows, source1.cols, CV_8UC3 );
	source1.convertTo( source1, CV_32F );
	source2.convertTo( source2, CV_32F );
	if ( style == huebright )
	{
		normalize( source1, source1, 0, 1, NORM_MINMAX );
		normalize( source2, source2, 0, 1, NORM_MINMAX );
		pow( source2, 2, source2 );
		for ( int r = 0; r < source1.rows; r++ )
		{
			for ( int c = 0; c < source1.cols; c++ )
			{
				auto BGRjet = colorMapJET( 255.*source1.at<float>( r, c ) );
				auto HUE = BGR_to_HUE( BGRjet );
				auto BGR = HUE_to_BGR( HUE, 255.*source2.at<float>( r, c ), BGRjet );

				result.at<Vec3b>( r, c )[0] = std::get<0>( BGR );
				result.at<Vec3b>( r, c )[1] = std::get<1>( BGR );
				result.at<Vec3b>( r, c )[2] = std::get<2>( BGR );
			}
		}
	}
	if ( style == bluered )
	{
		normalize( source2, source2, 0, 1, NORM_MINMAX );
		auto minMax = minMaxMat( source1 );
		for ( int r = 0; r < source1.rows; r++ )
		{
			for ( int c = 0; c < source1.cols; c++ )
			{
				auto BGR = colorMapBINARY( source1.at<float>( r, c ), std::get<0>( minMax ), std::get<1>( minMax ), sigma );
				result.at<Vec3b>( r, c )[0] = source2.at<float>( r, c ) * std::get<0>( BGR );
				result.at<Vec3b>( r, c )[1] = source2.at<float>( r, c ) * std::get<1>( BGR );
				result.at<Vec3b>( r, c )[2] = source2.at<float>( r, c ) * std::get<2>( BGR );
			}
		}
	}
	return result;
}

inline Mat applyQuantile( const Mat &sourceimgIn, double quantileB = 0, double quantileT = 1 )
{
	Mat sourceimg = sourceimgIn.clone();
	sourceimg.convertTo( sourceimg, CV_32FC1 );
	float caxisMin, caxisMax;
	std::tie( caxisMin, caxisMax ) = minMaxMat( sourceimg );

	if ( quantileB > 0 || quantileT < 1 )
	{
		std::vector<float> picvalues( sourceimg.rows * sourceimg.cols, 0 );
		for ( int r = 0; r < sourceimg.rows; r++ )
			for ( int c = 0; c < sourceimg.cols; c++ )
				picvalues[r * sourceimg.cols + c] = sourceimg.at<float>( r, c );

		sort( picvalues.begin(), picvalues.end() );
		caxisMin = picvalues[quantileB * ( picvalues.size() - 1 )];
		caxisMax = picvalues[quantileT * ( picvalues.size() - 1 )];
	}

	for ( int r = 0; r < sourceimg.rows; r++ )
		for ( int c = 0; c < sourceimg.cols; c++ )
			sourceimg.at<float>( r, c ) = clamp( sourceimg.at<float>( r, c ), caxisMin, caxisMax );

	return sourceimg;
}

inline Mat applyQuantileColorMap( const Mat &sourceimgIn, double quantileB = 0, double quantileT = 1 )
{
	Mat sourceimg = sourceimgIn.clone();
	sourceimg.convertTo( sourceimg, CV_32F );
	float caxisMin, caxisMax;
	std::tie( caxisMin, caxisMax ) = minMaxMat( sourceimg );

	if ( quantileB > 0 || quantileT < 1 )
	{
		std::vector<float> picvalues( sourceimg.rows * sourceimg.cols, 0 );
		for ( int r = 0; r < sourceimg.rows; r++ )
			for ( int c = 0; c < sourceimg.cols; c++ )
				picvalues[r * sourceimg.cols + c] = sourceimg.at<float>( r, c );

		sort( picvalues.begin(), picvalues.end() );
		caxisMin = picvalues[quantileB * ( picvalues.size() - 1 )];
		caxisMax = picvalues[quantileT * ( picvalues.size() - 1 )];
	}

	Mat sourceimgOutCLR( sourceimg.rows, sourceimg.cols, CV_32FC3 );
	for ( int r = 0; r < sourceimgOutCLR.rows; r++ )
	{
		for ( int c = 0; c < sourceimgOutCLR.cols; c++ )
		{
			float x = sourceimg.at<float>( r, c );
			float B, G, R;
			std::tie( B, G, R ) = colorMapJET( x, caxisMin, caxisMax );
			sourceimgOutCLR.at<Vec3f>( r, c )[0] = B;
			sourceimgOutCLR.at<Vec3f>( r, c )[1] = G;
			sourceimgOutCLR.at<Vec3f>( r, c )[2] = R;
		}
	}
	return sourceimgOutCLR;
}

inline void showimg( const Mat &sourceimgIn, std::string windowname, bool color = false, double quantileB = 0, double quantileT = 1, int wRows = 600 )
{
	Mat sourceimg = sourceimgIn.clone();

	double colRowRatio = ( double )sourceimg.cols / ( double )sourceimg.rows;
	int wCols = ( double )wRows * colRowRatio;
	namedWindow( windowname, WINDOW_NORMAL );
	resizeWindow( windowname, wCols, wRows );

	sourceimg.convertTo( sourceimg, CV_32F );
	normalize( sourceimg, sourceimg, 0, 1, NORM_MINMAX );

	if ( sourceimg.channels() == 1 )
	{
		if ( color )
			sourceimg = applyQuantileColorMap( sourceimg, quantileB, quantileT );
		else if ( quantileB != 0 || quantileT != 1 )
			sourceimg = applyQuantile( sourceimg, quantileB, quantileT );
	}

	imshow( windowname, sourceimg );
	waitKey( 1 );
}

inline void showimg( const std::vector<Mat> &sourceimgIns, std::string windowname, bool color = false, double quantileB = 0, double quantileT = 1, int wRows = 600 )
{
	// 1st image determines the main hconcat height
	int mainHeight = sourceimgIns[0].rows;
	std::vector<Mat> sourceimgs;
	sourceimgs.reserve( sourceimgIns.size() );
	for ( auto &srcimg : sourceimgIns )
	{
		if ( !srcimg.empty() )
		{
			auto img = srcimg.clone();
			resize( img, img, Size( ( double )mainHeight / img.rows * img.cols, mainHeight ) );
			normalize( img, img, 0, 255, NORM_MINMAX );
			img.convertTo( img, CV_8U );
			sourceimgs.emplace_back( img );
		}
	}

	Mat concatenated;
	hconcat( sourceimgs, concatenated );
	showimg( concatenated, windowname, color, quantileB, quantileT, wRows );
}

inline void saveimg( std::string path, const Mat &sourceimgIn, bool bilinear = false, Size size = Size( 0, 0 ) )
{
	Mat sourceimg = sourceimgIn.clone();

	if ( size != Size2i( 0, 0 ) )
	{
		if ( bilinear )
			resize( sourceimg, sourceimg, size, 0, 0, INTER_LINEAR );
		else
			resize( sourceimg, sourceimg, size, 0, 0, INTER_NEAREST );
	}
	imwrite( path, sourceimg );
}

inline void saveMatToCsv( const std::string &path, const Mat &matIn )
{
	std::ofstream listing( path, std::ios::out | std::ios::trunc );
	Mat mat = matIn.clone();
	mat.convertTo( mat, CV_32F );
	for ( int r = 0; r < mat.rows; r++ )
	{
		for ( int c = 0; c < mat.cols; c++ )
		{
			listing << mat.at<float>( r, c ) << ",";
		}
		listing << endl;
	}
}

inline Point2d median( std::vector<Point2d> &vec )
{
	//function changes the vec order, watch out
	std::sort( vec.begin(), vec.end(), []( Point2d a, Point2d b ) {return a.x < b.x; } );
	return vec[vec.size() / 2];
}

inline Mat vectToMat( std::vector<double> &vec )
{
	return Mat( vec ).reshape( 0, vec.size() );
}

inline std::vector<Mat> vect2ToMats( std::vector<std::vector<double>> &vec )
{
	std::vector<Mat> result( vec.size() );
	for ( int i = 0; i < vec.size(); i++ )
		result[i] = Mat( vec[i] ).reshape( 0, vec[i].size() );
	return result;
}

inline std::vector<double> mat1ToVect( const Mat &mat )
{
	std::vector<double> result( mat.rows, 0 );
	for ( int r = 0; r < mat.rows; r++ )
		result[r] = mat.at<float>( r, 0 );
	return result;
}

inline std::vector<std::vector<double>> matToVect2( const Mat &mat )
{
	std::vector<std::vector<double>> result = zerovect2( mat.rows, mat.cols );
	for ( int r = 0; r < mat.rows; r++ )
		for ( int c = 0; c < mat.cols; c++ )
			result[r][c] = mat.at<float>( r, c );
	return result;
}

inline void colorMapDebug( double sigmaa )
{
	int rows = 500;
	int cols = 1500;
	Mat colormap = Mat::zeros( rows, cols, CV_8UC3 );
	ColorMapStyle colormapStyle = CM_BINARY;
	for ( int r = 0; r < rows; r++ )
	{
		for ( int c = 0; c < cols; c++ )
		{
			std::tuple<int, int, int>BGR;

			if ( colormapStyle == CM_JET )
			{
				double x = 255.*c / ( cols - 1. );
				BGR = colorMapJET( x );
			}
			if ( colormapStyle == CM_BINARY )
			{
				double x = 2.*( ( double )c / ( cols - 1. ) ) - 1.;
				BGR = colorMapBINARY( x, -1, 1, sigmaa );
			}

			colormap.at<Vec3b>( r, c )[0] = std::get<0>( BGR );
			colormap.at<Vec3b>( r, c )[1] = std::get<1>( BGR );
			colormap.at<Vec3b>( r, c )[2] = std::get<2>( BGR );
		}
	}
	showimg( colormap, "colormap" );
}

inline void exportToMATLAB( const Mat &Zdata, double xmin, double xmax, double ymin, double ymax, std::string path )
{
	std::ofstream listing( path, std::ios::out | std::ios::trunc );
	listing << xmin << "," << xmax << "," << ymin << "," << ymax << endl;
	for ( int r = 0; r < Zdata.rows; r++ )
		for ( int c = 0; c < Zdata.cols; c++ )
			listing << Zdata.at<float>( r, c ) << endl;
}

inline std::string to_string( const Point2d &point )
{
	return std::string( "[" + to_string( point.x ) + "," + to_string( point.y ) + "]" );
}

inline Mat quadrantMask( int rows, int cols )
{
	Mat qmask = Mat::zeros( rows, cols, CV_32F );
	for ( int r = 0; r < rows; r++ )
	{
		for ( int c = 0; c < cols; c++ )
		{
			if ( r > ( rows / 2 ) && c < ( cols / 2 ) ) //3rd quadrant
			{
				qmask.at<float>( r, c ) = 1;
			}
		}
	}
	return qmask;
}

inline Mat matFromVector( std::vector<double> &vec, int cols )
{
	int rows = vec.size();
	Mat result = Mat::zeros( rows, cols, CV_32F );
	for ( int r = 0; r < rows; r++ )
	{
		for ( int c = 0; c < cols; c++ )
		{
			result.at<float>( r, c ) = vec[r];
		}
	}
	return result;
}

inline Mat matFromVector( const std::vector<std::vector<double>> &vec, bool transpose = false )
{
	if ( transpose )
	{
		int cols = vec.size();
		int rows = vec[0].size();
		Mat result = Mat::zeros( rows, cols, CV_32F );
		for ( int r = 0; r < rows; r++ )
		{
			for ( int c = 0; c < cols; c++ )
			{
				result.at<float>( r, c ) = vec[c][r];
			}
		}
		return result;
	}
	else
	{
		int rows = vec.size();
		int cols = vec[0].size();
		Mat result = Mat::zeros( rows, cols, CV_32F );
		for ( int r = 0; r < rows; r++ )
		{
			for ( int c = 0; c < cols; c++ )
			{
				result.at<float>( r, c ) = vec[r][c];
			}
		}
		return result;
	}
}

inline std::vector<double> meanHorizontal( const std::vector<std::vector<double>> &vec )
{
	std::vector<double> meansH( vec.size(), 0 );
	Mat mat = matFromVector( vec );

	for ( int r = 0; r < mat.rows; r++ )
	{
		for ( int c = 0; c < mat.cols; c++ )
		{
			meansH[r] += mat.at<float>( r, c );
		}
		meansH[r] /= mat.cols;
	}
	return meansH;
}

inline std::vector<double> meanVertical( const std::vector<std::vector<double>> &vec )
{
	std::vector<double> meansV( vec[0].size(), 0 );
	Mat mat = matFromVector( vec );

	for ( int c = 0; c < mat.cols; c++ )
	{
		for ( int r = 0; r < mat.rows; r++ )
		{
			meansV[c] += mat.at<float>( r, c );
		}
		meansV[c] /= mat.rows;
	}
	return meansV;
}

inline Point2f mean( const std::vector<Point2f> &vec )
{
	Point2f mean( 0, 0 );
	for ( auto &x : vec )
		mean += x;
	return mean * ( 1. / vec.size() );
}

inline Point2d mean( const std::vector<Point2d> &vec )
{
	Point2d mean( 0, 0 );
	for ( auto &x : vec )
		mean += x;
	return mean * ( 1. / vec.size() );
}

inline Point2f GetFeatureMatchShift( const DMatch &match, const std::vector<KeyPoint> &kp1, const std::vector<KeyPoint> &kp2 )
{
	return kp2[match.trainIdx].pt - kp1[match.queryIdx].pt;
}

inline std::pair<Point2f, Point2f> GetFeatureMatchPoints( const DMatch &match, const std::vector<KeyPoint> &kp1, const std::vector<KeyPoint> &kp2 )
{
	return std::make_pair( kp1[match.queryIdx].pt, kp2[match.trainIdx].pt );
}

inline double magnitude( const Point2f &pt )
{
	return sqrt( sqr( pt.x ) + sqr( pt.y ) );
}

inline double angle( const Point2f &pt )
{
	return toDegrees( atan2( pt.y, pt.x ) );
}

inline void drawPoint( Mat &out, const Point &pt, const Scalar &clr, int ptsz = 5, int thickness = 1 )
{
	Point ofst1( ptsz, ptsz );
	Point ofst2( ptsz, -ptsz );
	line( out, pt - ofst1, pt + ofst1, clr, thickness );
	line( out, pt - ofst2, pt + ofst2, clr, thickness );
}