#pragma once
#include "stdafx.h"
#include "functionsBaseSTL.h"
#include "logger.h"

#define ATTACH_CONSOLE//windows console for logging
#define LOGGER_QT//Qt text browser colorful logging
#define PLOTTER_QT//Qt QCustomPlot plotting
#define OPT_WITH_CV//additional optimization graphical functionality with OpenCV
//#define FOURIER_WITH_FFTW//optional use of the fftw library

constexpr LOGLEVEL g_loglevel = DEBUG;