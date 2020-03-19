#pragma once
#include "stdafx.h"
#include "Core/functionsBaseCV.h"
#include "Plot/Plot2D.h"

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
			Plot2D::plot( applyQuantile( FlowX, quanBot, quanTop ), "diffrot flow X", "solar longitude [pics]", "solar latitude [deg]", "horizontal plasma flow speed [deg/day]", 1, FlowX.cols, SourceThetas.front(), SourceThetas.back() );
		}

		// relative flow X jet
		if ( 1 )
		{
			showimg( FlowX - SourcePredicX, "diffrot relative flow X", true, quanBot, quanTop );
			Plot2D::plot( applyQuantile( FlowX - SourcePredicX, quanBot, quanTop ), "diffrot relative flow X", "solar longitude [pics]", "solar latitude [deg]", "relative horizontal plasma flow speed [deg/day]", 1, FlowX.cols, SourceThetas.front(), SourceThetas.back() );
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

	void SetData( std::vector<std::vector<double>> &image, std::vector<std::vector<double>> &flowX, std::vector<std::vector<double>> &predicX, std::vector<double> &thetas )
	{
		flip( matFromVector( image, true ), SourceImage, 1 );
		flip( matFromVector( flowX, true ), SourceFlowX, 1 );
		flip( matFromVector( predicX, true ), SourcePredicX, 1 );
		SourceThetas = ( 360. / Constants::TwoPi ) * thetas;
	}

private:
	Mat SourceImage;
	Mat SourceFlowX;
	Mat SourcePredicX;
	std::vector<double> SourceThetas;

	Mat FlowX;
};