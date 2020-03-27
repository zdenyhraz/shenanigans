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
		FlowY = SourceFlowY.clone();
		resize( FlowY, FlowY, FlowX.size() );

		if ( medianSize )
		{
			for ( int med = 3; med <= min( medianSize, 7 ); med += 2 )
			{
				medianBlur( FlowX, FlowX, med );
				medianBlur( FlowY, FlowY, med );
			}
		}

		magnitude( FlowX, FlowY, FlowM );
		phase( FlowX, FlowY, FlowP, true );

		double startTime = 0;
		double endTime = ( double )( SourcePics - 1 ) * SourceStride * 45 / 60 / 60 / 24;

		double startTheta = SourceThetas.front();
		double endTheta = SourceThetas.back();

		// diffrot profile X
		Plot1D::plot( SourceThetas, std::vector<std::vector<double>> {SourceOmegasXavg, SourceOmegasXavgpolyfit, SourceOmegasXavgsin2sin4fit, SourcePredicX1, SourcePredicX2}, "diffrot profile X", "solar latitude [deg]", "horizontal material flow speed [deg/day]", std::vector<std::string> {"omegasXavg", "omegasXavgpolyfit", "omegasXavgsin2sin4fit", "predicX1", "predicX2"}, saveDir + "1DXs" + to_string( SourceStride ) + ".png" );

		// diffrot profile Y
		Plot1D::plot( SourceThetas, std::vector<std::vector<double>> {SourceOmegasYavg, SourceOmegasYavgpolyfit}, "diffrot profile Y", "solar latitude [deg]", "vertical material flow speed [deg/day]", std::vector<std::string> {"omegasYavg", "omegasYavgpolyfit"}, saveDir + "1DYs" + to_string( SourceStride ) + ".png" );

		// shifts profile X
		Plot1D::plot( SourceThetas, std::vector<std::vector<double>> {SourceShiftsX, polyfit( SourceThetas, SourceShiftsX, 2 )}, "shifts profile X", "solar latitude [deg]", "horizontal shift [px]", std::vector<std::string> {"shiftsXavg", "shiftsXavgpolyfit"}, saveDir + "1DsXs" + to_string( SourceStride ) + ".png" );

		// shifts profile Y
		Plot1D::plot( SourceThetas, std::vector<std::vector<double>> {SourceShiftsY, polyfit( SourceThetas, SourceShiftsY, 3 )}, "shifts profile Y", "solar latitude [deg]", "vertical shift [px]", std::vector<std::string> {"shiftsYavg", "shiftsYavgpolyfit"}, saveDir + "1DsYs" + to_string( SourceStride ) + ".png" );

		// flow X
		Plot2D::plot( applyQuantile( FlowX, quanBot, quanTop ), "diffrot flow X", "time [days]", "solar latitude [deg]", "horizontal material flow speed [deg/day]", startTime, endTime, startTheta, endTheta, colRowRatio, saveDir + "2DXm" + to_string( medianSize ) + "s" + to_string( SourceStride ) + ".png" );

		// flow Y
		Plot2D::plot( applyQuantile( FlowY, quanBot, quanTop ), "diffrot flow Y", "time [days]", "solar latitude [deg]", "vertical material flow speed [deg/day]", startTime, endTime, startTheta, endTheta, colRowRatio, saveDir + "2DYm" + to_string( medianSize ) + "s" + to_string( SourceStride ) + ".png" );

		// flow magnitude
		Plot2D::plot( applyQuantile( FlowM, quanBot, quanTop ), "diffrot flow M", "time [days]", "solar latitude [deg]", "horizontal material flow speed magnitude [deg/day]", startTime, endTime, startTheta, endTheta, colRowRatio, saveDir + "2DMm" + to_string( medianSize ) + "s" + to_string( SourceStride ) + ".png" );

		// flow phase
		Plot2D::plot( applyQuantile( FlowP, quanBot, quanTop ), "diffrot flow P", "time [days]", "solar latitude [deg]", "horizontal material flow speed angle [deg]", startTime, endTime, startTheta, endTheta, colRowRatio, saveDir + "2DPm" + to_string( medianSize ) + "s" + to_string( SourceStride ) + ".png" );

		// relative flow X
		//Plot2D::plot( applyQuantile( FlowX - SourcePredicX, quanBot, quanTop ), "diffrot relative flow X", "time [days]", "solar latitude [deg]", "relative horizontal material flow speed [deg/day]", startTime, endTime, startTheta, endTheta, colRowRatio, saveDir + "2DrXm" + to_string( medianSize ) + "s" + to_string( SourceStride ) + ".png" );

		// relative flow binary X
		//showimg( combineTwoPics( FlowX - SourcePredicX, SourceImage, bluered, sigma ), "diffrot relative flow X binary", false, quanBot, quanTop );

	}

	void SetData2D( const std::vector<std::vector<double>> &image, const std::vector<std::vector<double>> &flowX, const std::vector<std::vector<double>> &flowY )
	{
		flip( matFromVector( image, true ), SourceImage, 1 );
		flip( matFromVector( flowX, true ), SourceFlowX, 1 );
		flip( matFromVector( flowY, true ), SourceFlowY, 1 );
	}

	void SetData1D( const std::vector<double> &thetas, const std::vector<double> &omegasXavg, const std::vector<double> &omegasYavg, const std::vector<double> &omegasXavgpolyfit, const std::vector<double> &omegasYavgpolyfit, const std::vector<double> &omegasXavgsin2sin4fit, const std::vector<double> &predicX1, const std::vector<double> &predicX2, const std::vector<double> &shiftsX, const std::vector<double> &shiftsY )
	{
		SourceThetas = ( 360. / Constants::TwoPi ) * thetas;
		SourceOmegasXavg = omegasXavg;
		SourceOmegasYavg = omegasYavg;
		SourceOmegasXavgpolyfit = omegasXavgpolyfit;
		SourceOmegasYavgpolyfit = omegasYavgpolyfit;
		SourceOmegasXavgsin2sin4fit = omegasXavgsin2sin4fit;
		SourcePredicX1 = predicX1;
		SourcePredicX2 = predicX2;
		SourceShiftsX = shiftsX;
		SourceShiftsY = shiftsY;
	}

	void SetParams( int pics, int stride )
	{
		SourcePics = pics;
		SourceStride = stride;
	}

private:

	// source data
	Mat SourceImage;
	Mat SourceFlowX;
	Mat SourceFlowY;
	int SourcePics;
	int SourceStride;
	std::vector<double> SourceThetas;
	std::vector<double> SourceOmegasXavg;
	std::vector<double> SourceOmegasYavg;
	std::vector<double> SourceOmegasXavgpolyfit;
	std::vector<double> SourceOmegasYavgpolyfit;
	std::vector<double> SourceOmegasXavgsin2sin4fit;
	std::vector<double> SourcePredicX1;
	std::vector<double> SourcePredicX2;
	std::vector<double> SourceShiftsX;
	std::vector<double> SourceShiftsY;

	// internal data
	Mat FlowX;
	Mat FlowY;
	Mat FlowM;
	Mat FlowP;
	std::string saveDir = "C:\\Users\\Zdeny\\Desktop\\PhD_things\\diffrot\\plotsave\\";
	static constexpr double colRowRatio = 2;
};