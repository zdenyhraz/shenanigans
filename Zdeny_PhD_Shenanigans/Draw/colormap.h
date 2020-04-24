#pragma once
#include "Core/functionsBaseCV.h"

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
