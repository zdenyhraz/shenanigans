#pragma once
#include "stdafx.h"
#include "IPC/IPC.h"
#include "Plot/plotterqt.h"
#include "Log/logger.h"

struct Globals
{
	IPCsettings* IPCset;
	QCustomPlot* widget1;
	QCustomPlot* widget2;
	Plotter2D* plotter2D;
	Logger* logger;

	Globals()
	{
		IPCset = new IPCsettings(100, 100, 5, 20);
		plotter2D = new Plotter2D();
		logger = new Logger();
	}
	
};
