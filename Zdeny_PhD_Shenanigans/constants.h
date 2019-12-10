#pragma once
#include "stdafx.h"
#include "functionsBaseSTL.h"
#include "logger.h"

#define ATTACH_CONSOLE//attach a console for logging (windows)
#define LOGGER_QT//Qt text browser colorful logging
#define PLOTTER_QT//Qt QCustomPlot plotting
#define OPTIMIZE_WITH_CV//additional optimization OpenCV graphical functionality
//#define FOURIER_WITH_FFTW//optional use of the fftw library
//#define FOURIER_WITH_CUDA//optional use of the cuda library

constexpr LOGLEVEL g_loglevel = DEBUG;