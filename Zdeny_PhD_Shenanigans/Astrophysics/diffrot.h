#pragma once
#include "stdafx.h"
#include "functionsAstro.h"
#include "diffrotResults.h"

static constexpr int plusminusbufer = 4;//even!
static constexpr int sy = 0;//sunspot shift
static constexpr int yshow = 400;//ipc show y index

struct DiffrotSettings
{
	int pics;
	int ys;
	int sPic;
	int dPic;
	int vFov;
	int dSec;
	bool medianFilter;
	bool movavgFilter;
	int medianFilterSize;
	int movavgFilterSize;
};

DiffrotResults calculateDiffrotProfile( const IPCsettings &ipcset, FitsTime &time, DiffrotSettings drset, IPlot1D *plt1, IPlot1D *plt2 );

void loadFitsFuzzy( FitsImage &pic, FitsTime &time );

void calculateOmegas( const FitsImage &pic1, const FitsImage &pic2, std::vector<double> &shiftsX, std::vector<double> &thetas, std::vector<double> &omegasX, std::vector<double> &image, std::vector<std::vector<double>> &predicXs, const IPCsettings &ipcset, const DiffrotSettings &drset, double R, double theta0, double dy );

std::vector<double> thetaFit( const std::vector<double> &omegas, const std::vector<double> &thetas );

void drplot1( IPlot1D *plt1, const std::vector<double> &thetas, const std::vector<double> &omegasX, const std::vector<std::vector<double>> &omegasX2D, const std::vector<double> &omegasXavgfit, const std::vector<std::vector<double>> &predicXs );

void drplot2( IPlot1D *plt2, const std::vector<double> &iotam, const std::vector<double> &shiftsX, const std::vector<double> &thetas );

void filterShiftsMEDIAN( std::vector<double> &shiftsX, int size );

void filterShiftsMOVAVG( std::vector<double> &shiftsX, int size );

double predictDiffrotProfile( double theta, double A, double B, double C );
