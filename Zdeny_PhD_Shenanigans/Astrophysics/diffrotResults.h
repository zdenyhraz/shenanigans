#pragma once
#include "stdafx.h"
#include "Core/functionsBaseCV.h"
#include "Plot/Plot1D.h"
#include "Plot/Plot2D.h"
#include "Fit/polyfit.h"
#include "Fit/trigfit.h"

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
		Plot1D::plot( SourceThetas, std::vector<std::vector<double>> {SourceOmegasXavg, SourceOmegasXavgpolyfit, SourcePredics[0], SourcePredics[1]}, "diffrot profile X", "solar latitude [deg]", "horizontal material flow speed [deg/day]", std::vector<std::string> {"average", "polyfit2", "Derek A. Lamb (2017)", "Howard et al. (1983)"}, saveDir + "1DXs" + to_string( SourceStride ) + ".png" );

		// diffrot profile Y
		Plot1D::plot( SourceThetas, std::vector<std::vector<double>> {SourceOmegasYavg, SourceOmegasYavgpolyfit}, "diffrot profile Y", "solar latitude [deg]", "vertical material flow speed [deg/day]", std::vector<std::string> {"average", "polyfit3"}, saveDir + "1DYs" + to_string( SourceStride ) + ".png" );

		// shifts profile X
		Plot1D::plot( SourceThetas, std::vector<std::vector<double>> {SourceShiftsX, polyfit( SourceThetas, SourceShiftsX, 2 )}, "shifts profile X", "solar latitude [deg]", "horizontal image shift [px]", std::vector<std::string> {"average", "polyfit2"}, saveDir + "1DsXs" + to_string( SourceStride ) + ".png" );

		// shifts profile Y
		Plot1D::plot( SourceThetas, std::vector<std::vector<double>> {SourceShiftsY, polyfit( SourceThetas, SourceShiftsY, 3 )}, "shifts profile Y", "solar latitude [deg]", "vertical image shift [px]", std::vector<std::string> {"average", "polyfit3"}, saveDir + "1DsYs" + to_string( SourceStride ) + ".png" );

		// flow X
		Plot2D::plot( applyQuantile( FlowX, quanBot, quanTop ), "diffrot flow X", "time [days]", "solar latitude [deg]", "horizontal material flow speed [deg/day]", startTime, endTime, startTheta, endTheta, colRowRatio, saveDir + "2DXm" + to_string( medianSize ) + "s" + to_string( SourceStride ) + ".png" );

		// flow Y
		Plot2D::plot( applyQuantile( FlowY, quanBot, quanTop ), "diffrot flow Y", "time [days]", "solar latitude [deg]", "vertical material flow speed [deg/day]", startTime, endTime, startTheta, endTheta, colRowRatio, saveDir + "2DYm" + to_string( medianSize ) + "s" + to_string( SourceStride ) + ".png" );
	}

	void SetData2D( const std::vector<std::vector<double>> &image, const std::vector<std::vector<double>> &flowX, const std::vector<std::vector<double>> &flowY )
	{
		flip( matFromVector( image, true ), SourceImage, 1 );
		flip( matFromVector( flowX, true ), SourceFlowX, 1 );
		flip( matFromVector( flowY, true ), SourceFlowY, 1 );
	}

	void SetData1D( const std::vector<double> &thetas, const std::vector<double> &omegasXavg, const std::vector<double> &omegasYavg, const std::vector<double> &omegasXavgpolyfit, const std::vector<double> &omegasYavgpolyfit, const std::vector<double> &omegasXavgsin2sin4fit, const std::vector<std::vector<double>> &predics, const std::vector<double> &shiftsX, const std::vector<double> &shiftsY )
	{
		SourceThetas = ( 360. / Constants::TwoPi ) * thetas;
		SourceOmegasXavg = omegasXavg;
		SourceOmegasYavg = omegasYavg;
		SourceOmegasXavgpolyfit = omegasXavgpolyfit;
		SourceOmegasYavgpolyfit = omegasYavgpolyfit;
		SourceOmegasXavgsin2sin4fit = omegasXavgsin2sin4fit;
		SourcePredics = predics;
		SourceShiftsX = shiftsX;
		SourceShiftsY = shiftsY;
	}

	void SetParams( int pics, int stride, std::string savepath )
	{
		SourcePics = pics;
		SourceStride = stride;
		saveDir = savepath;
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
	std::vector<std::vector<double>> SourcePredics;
	std::vector<double> SourceShiftsX;
	std::vector<double> SourceShiftsY;

	// internal data
	Mat FlowX;
	Mat FlowY;
	Mat FlowM;
	Mat FlowP;
	std::string saveDir;
	static constexpr double colRowRatio = 2;
};