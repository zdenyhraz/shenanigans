#pragma once
#include "stdafx.h"
#include "IPC/IPC.h"
#include "diffrotResults.h"

static constexpr int plusminusbufer = 4;//even!
static constexpr int sy = 0;//sunspot shift
static constexpr int yshow = 1400;//ipc show y index

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

DiffrotResults calculateDiffrotProfile( const IPCsettings &ipcset, FitsTime &time, DiffrotSettings drset );

void loadFitsFuzzy( FitsImage &pic, FitsTime &time );

void calculateOmegas( const FitsImage &pic1, const FitsImage &pic2, std::vector<double> &shiftsX, std::vector<double> &thetas, std::vector<double> &omegasX, std::vector<double> &image, std::vector<std::vector<double>> &predicXs, const IPCsettings &ipcset, const DiffrotSettings &drset, double R, double theta0, double dy );

double predictDiffrotProfile( double theta, double A, double B, double C );
