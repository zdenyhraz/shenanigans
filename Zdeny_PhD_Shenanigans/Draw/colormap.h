#pragma once
#include "Core/functionsBaseCV.h"

inline std::tuple<float, float, float> colorMapJET( float x, float caxisMin = 0, float caxisMax = 1, float val = 1 )
{
	if ( x == 0 )
		return std::make_tuple( 0., 0., 0. );

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

inline Scalar colorMapJet( float x, float caxisMin = 0, float caxisMax = 1, float val = 255 )
{
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
