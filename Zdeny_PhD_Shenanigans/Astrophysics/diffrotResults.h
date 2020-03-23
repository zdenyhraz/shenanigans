#pragma once
#include "stdafx.h"
#include "Core/functionsBaseCV.h"
#include "Plot/Plot1D.h"
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

		// diffrot profile
		if ( 1 )
		{
			Plot1D::plot( SourceThetas, std::vector<std::vector<double>> {SourceOmegasXavg, SourceOmegasXavgfit, SourcePredicX1, SourcePredicX2}, "diffrot profile X", "solar latitude [deg]", "horizontal plasma flow speed [deg/day]", std::vector<std::string> {"omegasXavg", "omegasXavgfit", "predicX1", "predicX2"} );
		}

		// source img
		if ( 1 )
		{
			showimg( SourceImage, "diffrot source" );
		}

		// flow X jet
		if ( 1 )
		{
			Plot2D::plot( applyQuantile( FlowX, quanBot, quanTop ), "diffrot flow X", "solar longitude [pics]", "solar latitude [deg]", "horizontal plasma flow speed [deg/day]", 1, FlowX.cols, SourceThetas.front(), SourceThetas.back(), colRowRatio, saveDir + "diffrot flow X.png" );
		}

		// relative flow X jet
		if ( 1 )
		{
			Plot2D::plot( applyQuantile( FlowX - SourcePredicX, quanBot, quanTop ), "diffrot relative flow X", "solar longitude [pics]", "solar latitude [deg]", "relative horizontal plasma flow speed [deg/day]", 1, FlowX.cols, SourceThetas.front(), SourceThetas.back(), colRowRatio, saveDir + "diffrot relative flow X.png" );
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

	void SetData2D( std::vector<std::vector<double>> &image, std::vector<std::vector<double>> &flowX, std::vector<std::vector<double>> &predicX )
	{
		flip( matFromVector( image, true ), SourceImage, 1 );
		flip( matFromVector( flowX, true ), SourceFlowX, 1 );
		flip( matFromVector( predicX, true ), SourcePredicX, 1 );
	}

	void SetData1D( std::vector<double> &thetas, std::vector<double> &omegasXavg, std::vector<double> &omegasXavgfit, std::vector<double> &predicX1, std::vector<double> &predicX2 )
	{
		SourceThetas = ( 360. / Constants::TwoPi ) * thetas;
		SourceOmegasXavg = omegasXavg;
		SourceOmegasXavgfit = omegasXavgfit;
		SourcePredicX1 = predicX1;
		SourcePredicX2 = predicX2;
	}

private:
	Mat SourceImage;
	Mat SourceFlowX;
	Mat SourcePredicX;

	std::vector<double> SourceThetas;
	std::vector<double> SourceOmegasXavg;
	std::vector<double> SourceOmegasXavgfit;
	std::vector<double> SourcePredicX1;
	std::vector<double> SourcePredicX2;

	Mat FlowX;
	std::string saveDir = "C:\\Users\\Zdeny\\Desktop\\PhD_things\\diffrot\\plotsave\\";
	static constexpr double colRowRatio = 2;
};