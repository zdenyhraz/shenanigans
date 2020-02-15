#pragma once
#include "macros.h"
#include "IPC/IPC.h"
#include "Log/loggercsl.h"
#include "Log/loggerqt.h"
#include "Plot/plotterqt.h"

struct Globals
{
	IPCsettings* IPCsettings;
	Logger* Logger;
	QCustomPlot* widget1;
	QCustomPlot* widget2;
	Plotter2D* plotter2D;
};
