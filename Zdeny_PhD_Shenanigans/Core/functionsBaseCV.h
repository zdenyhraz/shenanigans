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

inline double magnitude( const Point2f &pt )
{
	return sqrt( sqr( pt.x ) + sqr( pt.y ) );
}

inline double angle( const Point2f &pt )
{
	return toDegrees( atan2( pt.y, pt.x ) );
}

inline std::pair<double, double> minMaxMat( const Mat &sourceimg )
{
	double minR, maxR;
	minMaxLoc( sourceimg, &minR, &maxR, nullptr, nullptr );
	return make_pair( minR, maxR );
}

inline std::string to_string( const Point2d &point )
{
	return std::string( "[" + to_string( point.x ) + "," + to_string( point.y ) + "]" );
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

inline Point2d median( std::vector<Point2d> &vec )
{
	//function changes the vec order, watch out
	std::sort( vec.begin(), vec.end(), []( Point2d a, Point2d b ) {return a.x < b.x; } );
	return vec[vec.size() / 2];
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

inline Point2f mean( const std::vector<Point2f> &vec )
{
	Point2f mean( 0, 0 );
	for ( auto &x : vec )
		mean += x;
	return mean * ( 1. / vec.size() );
}
