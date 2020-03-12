#pragma once
#include "stdafx.h"

class DiffrotResults
{
public:
	void ShowResults( int medianSize, double sigma, double quanBot = 0, double quanTop = 1 )
	{
		// reset
		FlowX = SourceFlowX.clone();

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

		// flow X jet
		if ( 1 )
		{
			showimg( FlowX, "diffrot flow X", true, quanBot, quanTop );
		}

		// relative flow X jet
		if ( 1 )
		{
			showimg( FlowX - SourcePredicX, "diffrot relative flow X", true, quanBot, quanTop );
		}

		// relative flow X binary
		if ( 1 )
		{
			showimg( combineTwoPics( FlowX - SourcePredicX, SourceImage, binary, sigma ), "diffrot relative flow X binary", false, quanBot, quanTop );
		}

		// relative magnitude & absolute phase
		if ( 1 )
		{
			// needs Y information
		}
	}

	void SetData( Mat &image, Mat &flowX, Mat &predicX )
	{
		flip( image, SourceImage, 1 );
		flip( flowX, SourceFlowX, 1 );
		flip( predicX, SourcePredicX, 1 );

		FlowX = SourceFlowX;
		FlowX.convertTo( FlowX, CV_32F );
	}

private:
	Mat SourceImage;
	Mat SourceFlowX;
	Mat SourcePredicX;

	Mat FlowX;
};