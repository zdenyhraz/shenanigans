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

inline void updateHighest( std::vector<std::pair<int, double>> &proxidxs, int idx, double val )
{
	int maxidx = 0;
	double maxval = 0;
	for ( int i = 0; i < proxidxs.size(); i++ )
	{
		if ( proxidxs[i].second > maxval )
		{
			maxval = proxidxs[i].second;
			maxidx = i;
		}
	}

	if ( proxidxs[maxidx].second > val )
	{
		proxidxs[maxidx] = std::make_pair( idx, val );
	}
}

inline Mat wnnfit( const std::vector<Point2f> &pts, const std::vector<double> &zdata, double xmin, double xmax, double ymin, double ymax, int xcnt, int ycnt, int proxpts = 10, double proxcoeff = 7 )
{
	Mat out = Mat::zeros( ycnt, xcnt, CV_32F );
	proxpts = min( proxpts, ( int )pts.size() );
	for ( int r = 0; r < out.rows; r++ )
	{
		for ( int c = 0; c < out.cols; c++ )
		{
			//get correspoinding point values
			double x = xmin + ( ( float )c / ( out.cols - 1 ) ) * ( xmax - xmin );
			double y = ymin + ( ( float )r / ( out.rows - 1 ) ) * ( ymax - ymin );
			Point2f pt( x, y );

			//get first proxpts point indices, maxdistance
			double distance = 0;
			double maxdistance = 0;
			std::vector<std::pair<int, double>> proxidxs( proxpts );
			for ( int i = 0; i < pts.size(); i++ )
			{
				distance = magnitude( pts[i] - pt );
				if ( distance > maxdistance )
					maxdistance = distance;

				if ( i < proxpts )
					proxidxs.push_back( std::make_pair( i, distance ) );
				else
					updateHighest( proxidxs, i, distance );
			}

			//weighted average
			double weight = 0;
			double weightsum = 0;
			for ( auto i : proxidxs )
			{
				weight = max( 1. - magnitude( pts[i.first] - pt ) / ( maxdistance / proxcoeff ), 0. );
				weightsum += weight;
				out.at<float>( r, c ) += weight * zdata[i.first];
			}
			out.at<float>( r, c ) /= weightsum;
		}
	}
	return out;
}