#pragma once
#include "stdafx.h"
#include "baseFunctions2d.h"

using namespace std;
using namespace cv;
using namespace Procedural;

inline Mat procedural( int rows, int cols )
{
	int N = 500;
	Mat mat = Mat::zeros( rows, cols, CV_32F );

	for ( int i = 0; i < N; i++ )
	{
		float cx = 0.02 * rand01();
		float cy = 0.02 * rand01();
		float ratio = cx / cy ;

		if ( ratio > 5 || ratio < ( 1. / 5 ) )
			continue;

		mat += ( rand01() * gaussian( rows, cols, cx, cy, rand01(), rand01() ) );
	}
	normalize( mat, mat, 0, 1, NORM_MINMAX );
	return mat;
}

inline Mat colorlandscape( const Mat &heightmap )
{
	Mat mat = Mat::zeros( heightmap.rows, heightmap.cols, CV_32FC3 );

	for ( int r = 0; r < heightmap.rows; r++ )
	{
		for ( int c = 0; c < heightmap.cols; c++ )
		{
			auto &x = heightmap.at<float>( r, c );

			if ( x < 0.23 )
			{
				//deep water
				mat.at<Vec3f>( r, c )[0] = 255. / 255;
				mat.at<Vec3f>( r, c )[1] = 0. / 255;
				mat.at<Vec3f>( r, c )[2] = 0. / 255;
			}
			else if ( x < 0.35 )
			{
				//shallow water
				mat.at<Vec3f>( r, c )[0] = 255. / 255;
				mat.at<Vec3f>( r, c )[1] = 191. / 255;
				mat.at<Vec3f>( r, c )[2] = 0. / 255;
			}
			else if ( x < 0.43 )
			{
				//sandy beaches
				mat.at<Vec3f>( r, c )[0] = 62. / 255;
				mat.at<Vec3f>( r, c )[1] = 226. / 255;
				mat.at<Vec3f>( r, c )[2] = 254. / 255;
			}
			else if ( x < 0.55 )
			{
				//grass lands
				mat.at<Vec3f>( r, c )[0] = 50. / 255;
				mat.at<Vec3f>( r, c )[1] = 205. / 255;
				mat.at<Vec3f>( r, c )[2] = 50. / 255;
			}
			else if ( x < 0.65 )
			{
				//forest
				mat.at<Vec3f>( r, c )[0] = 0. / 255;
				mat.at<Vec3f>( r, c )[1] = 128. / 255;
				mat.at<Vec3f>( r, c )[2] = 0. / 255;
			}
			else if ( x < 0.73 )
			{
				//dirt
				mat.at<Vec3f>( r, c )[0] = 19. / 255;
				mat.at<Vec3f>( r, c )[1] = 69. / 255;
				mat.at<Vec3f>( r, c )[2] = 139. / 255;
			}
			else if ( x < 0.87 )
			{
				//rocky hills
				mat.at<Vec3f>( r, c )[0] = 0.5;
				mat.at<Vec3f>( r, c )[1] = 0.5;
				mat.at<Vec3f>( r, c )[2] = 0.5;
			}
			else
			{
				//snow
				mat.at<Vec3f>( r, c )[0] = 0.88;
				mat.at<Vec3f>( r, c )[1] = 0.88;
				mat.at<Vec3f>( r, c )[2] = 0.88;
			}
		}
	}
	return mat;
}

