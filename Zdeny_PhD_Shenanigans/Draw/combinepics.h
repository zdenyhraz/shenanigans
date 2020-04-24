#pragma once
#include "Core/functionsBaseCV.h"
#include "Draw/colormap.h"

enum CombineType
{
	COMBINE_JET,
	COMBINE_BIN
};

inline Mat combinePics( const Mat &img1In, const Mat &img2In, CombineType ctype, double quanB = 0, double quanT = 1 )
{
	Mat out = Mat::zeros( img1In.rows, img1In.cols, CV_32FC3 );
	Mat img1 = img1In.clone();
	Mat img2 = img2In.clone();
	resize( img2, img2, Size( img1.cols, img1.rows ) );
	img1.convertTo( img1, CV_32F );
	img2.convertTo( img2, CV_32F );
	normalize( img1, img1, 0, 1, NORM_MINMAX );
	normalize( img2, img2, 0, 1, NORM_MINMAX );
	img1 = applyQuantile( img1, quanB, quanT );
	img2 = applyQuantile( img2, quanB, quanT );

	switch ( ctype )
	{
		case COMBINE_JET:
		{
			for ( int r = 0; r < out.rows; r++ )
			{
				for ( int c = 0; c < out.cols; c++ )
				{
					float x1 = img1.at<float>( r, c );
					float x2 = img2.at<float>( r, c );
					Scalar jet = colorMapJet( x2, 0, 1, 1 );
					out.at<Vec3f>( r, c )[0] = x1 * jet[0];
					out.at<Vec3f>( r, c )[1] = x1 * jet[1];
					out.at<Vec3f>( r, c )[2] = x1 * jet[2];
				}
			}
			break;
		}
		case COMBINE_BIN:
		{

			break;
		}
	}

	return out;
}
