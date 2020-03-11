#pragma once
#include "stdafx.h"

class DiffrotResults
{
public:
	void ShowResults( int medianSize, double sigma, double quanBot = 0, double quanTop = 1 )
	{
		if ( medianSize )
		{
			for ( int med = 3; med <= min( medianSize, 7 ); med += 2 )
			{
				medianBlur( FlowX, FlowX, med );
			}
		}

		// source img
		if ( 1 )
		{
			showimg( SourceImage, "diffrot source" );
		}

		// flow X
		if ( 1 )
		{
			showimg( FlowX, "diffrot flow X", true, quanBot, quanTop );
		}
	}

	void SetData( Mat &image, Mat &flowX )
	{
		flip( image, SourceImage, 1 );
		flip( flowX, SourceFlowX, 1 );

		FlowX = SourceFlowX;
		FlowX.convertTo( FlowX, CV_32F );
	}

private:
	Mat SourceImage;
	Mat SourceFlowX;
	Mat FlowX;
};