#pragma once
#include "Core/constants.h"
#include "Core/functionsBaseCV.h"

inline int findNearestNeighborIndex( const std::vector<Point2f> &pts, Point2f pt )
{
	int idx = 0;
	double mindist = Constants::Max;
	double dist;
	for ( int i = 0; i < pts.size(); i++ )
	{
		dist = magnitude( pts[i] - pt );
		if ( dist < mindist )
		{
			mindist = dist;
			idx = i;
		}
	}
	return idx;
}

inline Mat nnfit( const std::vector<Point2f> &pts, const std::vector<double> &zdata, double xmin, double xmax, double ymin, double ymax, int xcnt, int ycnt )
{
	Mat out = Mat::zeros( ycnt, xcnt, CV_32F );
	for ( int r = 0; r < out.rows; r++ )
	{
		for ( int c = 0; c < out.cols; c++ )
		{
			double x = xmin + ( ( float )c / ( out.cols - 1 ) ) * ( xmax - xmin );
			double y = ymin + ( ( float )r / ( out.rows - 1 ) ) * ( ymax - ymin );
			Point2f pt( x, y );
			out.at<float>( r, c ) = zdata[findNearestNeighborIndex( pts, pt )];
		}
	}
	return out;
}