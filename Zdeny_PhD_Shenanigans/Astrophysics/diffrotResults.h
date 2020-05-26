#pragma once
#include "stdafx.h"
#include "Core/functionsBaseCV.h"
#include "Plot/Plot1D.h"
#include "Plot/Plot2D.h"
#include "Fit/polyfit.h"
#include "Fit/trigfit.h"

inline double predictDiffrotProfile( double theta, double A, double B, double C = 0 )
{
	return ( A + B * pow( sin( theta ), 2 ) + C * pow( sin( theta ), 4 ) );
}

class DiffrotResults
{
public:
	void ShowResults( int medianSize, double sigma, double quanBot = 0, double quanTop = 1 )
	{
		Reset();

		CalculateMedianFilters( medianSize );
		CalculateMagnitudeAndPhase();
		CalculatePredics();
		CalculateAxisLimits();
		CalculateErrors();
		CalculateNS();

		// ======================================================== PLOTS ========================================================

		// diffrot profiles
		Plot1D::plot( SourceThetasavg, std::vector<std::vector<double>> {SourceOmegasXavg, SourceOmegasXavgpolyfit, predicXs[0], predicXs[1]}, "diffrot profile X", "solar latitude [deg]", "horizontal material flow speed [deg/day]", std::vector<std::string> {"average", "polyfit2", "Derek A. Lamb (2017)", "Howard et al. (1983)"}, std::vector<QPen> { QPen( Qt::blue, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::darkGreen, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::red, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin )}, saveDir + "1DXs" + to_string( SourceStride ) + ".png" );
		Plot1D::plot( SourceThetasavg, std::vector<std::vector<double>> {SourceOmegasYavg, SourceOmegasYavgpolyfit}, "diffrot profile Y", "solar latitude [deg]", "vertical material flow speed [deg/day]", std::vector<std::string> {"average", "polyfit3"}, std::vector<QPen> { QPen( Qt::blue, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::darkGreen, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin )}, saveDir + "1DYs" + to_string( SourceStride ) + ".png" ); //rgb(119, 136, 153)

		// diffrot profiles NS
		Plot1D::plot( ThetasNS, std::vector<std::vector<double>> {OmegasXavgN, OmegasXavgS, polyfit( OmegasXavgN, 2 ), polyfit( OmegasXavgS, 2 )}, "diffrot profile NS X", "absolute solar latitude [deg]", "horizontal material flow speed [deg/day]", std::vector<std::string> {"averageN", "averageS", "polyfit2N", "polyfit2S" }, std::vector<QPen> { QPen( Qt::blue, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::black, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::blue, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::black, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin )}, saveDir + "1DNSXs" + to_string( SourceStride ) + ".png" );
		Plot1D::plot( ThetasNS, std::vector<std::vector<double>> {OmegasYavgN, OmegasYavgS, polyfit( OmegasYavgN, 3 ), polyfit( OmegasYavgS, 3 )}, "diffrot profile NS Y", "absolute solar latitude [deg]", "absolute vertical material flow speed [deg/day]", std::vector<std::string> {"averageN", "averageS", "polyfit3N", "polyfit3S" }, std::vector<QPen> { QPen( Qt::blue, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::black, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::blue, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::black, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin )}, saveDir + "1DNSYs" + to_string( SourceStride ) + ".png" );

		// shifts profiles
		Plot1D::plot( SourceThetasavg, std::vector<std::vector<double>> {SourceShiftsXavg, polyfit( SourceThetasavg, SourceShiftsXavg, 2 ), shiftsXErrorsBot, shiftsXErrorsTop}, "shifts profile X", "solar latitude [deg]", "horizontal image shift [px]", std::vector<std::string> {"average", "polyfit2", "average - 3SEM", "average + 3SEM"}, std::vector<QPen> { QPen( Qt::blue, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::darkGreen, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::cyan, 0.75, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::magenta, 0.75, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin )}, saveDir + "1DsXs" + to_string( SourceStride ) + ".png" );
		Plot1D::plot( SourceThetasavg, std::vector<std::vector<double>> {SourceShiftsYavg, polyfit( SourceThetasavg, SourceShiftsYavg, 3 ), shiftsYErrorsBot, shiftsYErrorsTop}, "shifts profile Y", "solar latitude [deg]", "vertical image shift [px]", std::vector<std::string> {"average", "polyfit3", "average - 3SEM", "average + 3SEM"}, std::vector<QPen> { QPen( Qt::blue, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::darkGreen, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::cyan, 0.75, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::magenta, 0.75, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin )}, saveDir + "1DsYs" + to_string( SourceStride ) + ".png" );

		// shifts mean errors
		//Plot1D::plot( SourceThetasavg, shiftsXErrors, "shifts X errors", "solar latitude [deg]", "horizontal image shift error[px]", std::vector<QPen> { QPen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin )}, saveDir + "1DeXs" + to_string( SourceStride ) + ".png" );
		//Plot1D::plot( SourceThetasavg, shiftsYErrors, "shifts Y errors", "solar latitude [deg]", "vertical image shift error[px]", std::vector<QPen> { QPen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin )}, saveDir + "1DeYs" + to_string( SourceStride ) + ".png" );

		// flows
		Plot2D::plot( applyQuantile( FlowX, quanBot, quanTop ), "diffrot flow X", "time [days]", "solar latitude [deg]", "horizontal material flow speed [deg/day]", startTime, endTime, startTheta, endTheta, colRowRatio, saveDir + "2DXm" + to_string( medianSize ) + "s" + to_string( SourceStride ) + ".png" );
		Plot2D::plot( applyQuantile( FlowY, quanBot, quanTop ), "diffrot flow Y", "time [days]", "solar latitude [deg]", "vertical material flow speed [deg/day]", startTime, endTime, startTheta, endTheta, colRowRatio, saveDir + "2DYm" + to_string( medianSize ) + "s" + to_string( SourceStride ) + ".png" );
	}

	void SetData2D( const std::vector<std::vector<double>> &image, const std::vector<std::vector<double>> &flowX, const std::vector<std::vector<double>> &flowY, const std::vector<std::vector<double>> &shiftsX, const std::vector<std::vector<double>> &shiftsY )
	{
		flip( matFromVector( image, true ), SourceImage, 1 );
		flip( matFromVector( flowX, true ), SourceFlowX, 1 );
		flip( matFromVector( flowY, true ), SourceFlowY, 1 );
		SourceShiftsX = shiftsX;
		SourceShiftsY = shiftsY;
	}

	void SetData1D( const std::vector<double> &thetasavg, const std::vector<double> &omegasXavg, const std::vector<double> &omegasYavg, const std::vector<double> &omegasXavgpolyfit, const std::vector<double> &omegasYavgpolyfit, const std::vector<double> &shiftsXavg, const std::vector<double> &shiftsYavg )
	{
		SourceThetasavg = ( 360. / Constants::TwoPi ) * thetasavg;
		SourceOmegasXavg = omegasXavg;
		SourceOmegasYavg = omegasYavg;
		SourceOmegasXavgpolyfit = omegasXavgpolyfit;
		SourceOmegasYavgpolyfit = omegasYavgpolyfit;
		SourceShiftsXavg = shiftsXavg;
		SourceShiftsYavg = shiftsYavg;
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
	std::vector<std::vector<double>> SourceShiftsX;
	std::vector<std::vector<double>> SourceShiftsY;
	std::vector<double> SourceThetasavg;
	std::vector<double> SourceOmegasXavg;
	std::vector<double> SourceOmegasYavg;
	std::vector<double> SourceOmegasXavgpolyfit;
	std::vector<double> SourceOmegasYavgpolyfit;
	std::vector<double> SourceShiftsXavg;
	std::vector<double> SourceShiftsYavg;

	// internal data
	Mat FlowX;
	Mat FlowY;
	Mat FlowM;
	Mat FlowP;
	std::string saveDir;
	static constexpr double colRowRatio = 2;
	std::vector<std::vector<double>> predicXs;
	std::vector<double> shiftsXErrors;
	std::vector<double> shiftsYErrors;
	std::vector<double> shiftsXErrorsBot;
	std::vector<double> shiftsXErrorsTop;
	std::vector<double> shiftsYErrorsBot;
	std::vector<double> shiftsYErrorsTop;

	std::vector<std::vector<double>> predicXsN;
	std::vector<std::vector<double>> predicXsS;
	std::vector<double> ThetasNS;
	std::vector<double> OmegasXavgN;
	std::vector<double> OmegasXavgS;
	std::vector<double> OmegasYavgN;
	std::vector<double> OmegasYavgS;

	double startTime;
	double endTime;
	double startTheta;
	double endTheta;

	void Reset()
	{
		FlowX = SourceFlowX.clone();
		FlowY = SourceFlowY.clone();
		resize( FlowY, FlowY, FlowX.size() );
	}

	void CalculateMedianFilters( int medianSize )
	{
		if ( medianSize )
		{
			for ( int med = 3; med <= min( medianSize, 7 ); med += 2 )
			{
				medianBlur( FlowX, FlowX, med );
				medianBlur( FlowY, FlowY, med );
			}
		}
	}

	void CalculateMagnitudeAndPhase()
	{
		magnitude( FlowX, FlowY, FlowM );
		phase( FlowX, FlowY, FlowP, true );
	}

	void CalculateAxisLimits()
	{
		startTime = 0;
		endTime = ( double )( SourcePics - 1 ) * SourceStride * 45 / 60 / 60 / 24;
		startTheta = SourceThetasavg.front();
		endTheta = SourceThetasavg.back();
	}

	void CalculatePredics()
	{
		predicXs = zerovect2( 2, SourceThetasavg.size() );
		for ( int i = 0; i < SourceThetasavg.size(); i++ )
		{
			predicXs[0][i] = predictDiffrotProfile( SourceThetasavg[i], 14.296, -1.847, -2.615 ); //Derek A. Lamb (2017)
			predicXs[1][i] = predictDiffrotProfile( SourceThetasavg[i], 14.192, -1.70, -2.36 ); //Howard et al. (1983)
			// etc...
		}
	}

	void CalculateErrors()
	{
		shiftsXErrors = getStandardErrorsOfTheMeanVertical( SourceShiftsX );
		shiftsYErrors = getStandardErrorsOfTheMeanVertical( SourceShiftsY );
		shiftsXErrorsBot = SourceShiftsXavg - 3 * shiftsXErrors;
		shiftsYErrorsBot = SourceShiftsYavg - 3 * shiftsYErrors;
		shiftsXErrorsTop = SourceShiftsXavg + 3 * shiftsXErrors;
		shiftsYErrorsTop = SourceShiftsYavg + 3 * shiftsYErrors;
	}

	void CalculateNS()
	{
		int zeroidx = 0;
		for ( int i = 0; i < SourceThetasavg.size() - 1; i++ )
		{
			if ( ( SourceThetasavg[i] > 0 && SourceThetasavg[i + 1] < 0 ) || SourceThetasavg[i] == 0 )
			{
				zeroidx = i + 1;
				LOG_DEBUG( "Diffrot NS zeroidx = {} / {}", zeroidx, SourceThetasavg.size() );
				break;
			}
		}

		//north hemisphere
		ThetasNS = std::vector<double>( SourceThetasavg.begin(), SourceThetasavg.begin() + zeroidx + 1 );
		OmegasXavgN = std::vector<double>( SourceOmegasXavg.begin(), SourceOmegasXavg.begin() + zeroidx + 1 );
		OmegasYavgN = abs( std::vector<double>( SourceOmegasYavg.begin(), SourceOmegasYavg.begin() + zeroidx + 1 ) );

		//south hemisphere
		OmegasXavgS = std::vector<double>( SourceOmegasXavg.begin() + zeroidx, SourceOmegasXavg.begin() + 2 * zeroidx + 1 );
		OmegasYavgS = abs( std::vector<double>( SourceOmegasYavg.begin() + zeroidx, SourceOmegasYavg.begin() + 2 * zeroidx + 1 ) );
		std::reverse( OmegasXavgS.begin(), OmegasXavgS.end() );
		std::reverse( OmegasYavgS.begin(), OmegasYavgS.end() );

		//cut off stuff near the equator for both hemispheres for nicer poly2 fit
		double offs = 10;//deg
		int offsidx = 0;
		for ( int i = 0; i < ThetasNS.size() - 1; i++ )
		{
			if ( ThetasNS[i] < offs )
			{
				offsidx = i;
				break;
			}
		}

		ThetasNS = std::vector<double>( ThetasNS.begin(), ThetasNS.begin() + offsidx );
		OmegasXavgN = std::vector<double>( OmegasXavgN.begin(), OmegasXavgN.begin() + offsidx );
		OmegasYavgN = std::vector<double>( OmegasYavgN.begin(), OmegasYavgN.begin() + offsidx );
		OmegasXavgS = std::vector<double>( OmegasXavgS.begin(), OmegasXavgS.begin() + offsidx );
		OmegasYavgS = std::vector<double>( OmegasYavgS.begin(), OmegasYavgS.begin() + offsidx );
	}

};
