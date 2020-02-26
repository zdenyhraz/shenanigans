#pragma once
#include "stdafx.h"
#include "functionsAstro.h"

static constexpr int plusminusbufer = 4;//even!
static constexpr int sy = 0;//sunspot shift
constexpr int ma = 5;//moving average window

struct DiffrotSettings
{
	int pics;
	int ys;
	int sPic;
	int dPic;
	int vFov;
	int dSec;
};

DiffrotResults calculateDiffrotProfile( const IPCsettings &ipcset, FitsTime &time, DiffrotSettings drset, IPlot1D *plt1, IPlot1D *plt2 );

void loadFitsFuzzy( FitsImage &pic, FitsTime &time );

void calculateOmegas( const FitsImage &pic1, const FitsImage &pic2, std::vector<double> &shiftsX, std::vector<double> &thetas, std::vector<double> &omegasX,
                      std::vector<double> &predicX, const IPCsettings &ipcset, const DiffrotSettings &drset, double R, double theta0, double dy );

DiffrotResults fillDiffrotResults();

std::vector<double> theta1Dfit( const std::vector<double> &omegas, const std::vector<double> &thetas );

std::vector<double> theta2Dfit( const std::vector<std::vector<double>> &omegasX2D, const std::vector<std::vector<double>> &thetas2D );

void drplot1( IPlot1D *plt1, const std::vector<double> &thetas, const std::vector<double> &omegasX, const std::vector<double> &omegasXfit, const std::vector<double> &omegasXavgfit,
              const std::vector<double> &predicX );

void drplot2( IPlot1D *plt2, const std::vector<double> &thetas, const std::vector<double> &shiftsX );

void filterShiftsMA( std::vector<double> &shiftsX );

std::vector<double> predictions( double theta );