#pragma once
#include "Core/functionsBaseCV.h"
#include "Draw/colormap.h"

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
