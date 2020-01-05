#pragma once
#include "constants.h"
#include "IPC.h"
#include "loggercsl.h"
#include "loggerqt.h"
#include "plotterqt.h"

struct Globals
{
	IPCsettings* IPCsettings;
	Logger* Logger;
	QCustomPlot* widget1;
	QCustomPlot* widget2;
	Plotter2D* plotter2D;
};
