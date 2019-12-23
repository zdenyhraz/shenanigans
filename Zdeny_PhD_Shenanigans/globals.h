#pragma once
#include "constants.h"
#include "IPC.h"
#include "loggercsl.h"
#include "loggerqt.h"
#include "plotterqt.h"
#include "WindowPlot.h"

struct Globals
{
	IPCsettings* IPCsettings;
	Logger* Logger;
	QCustomPlot* widget;
	std::vector<WindowPlot*> windowPlots;
};
