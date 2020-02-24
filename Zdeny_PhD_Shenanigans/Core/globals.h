#pragma once
#include "stdafx.h"
#include "IPC/IPC.h"
#include "Plot/plotterqt.h"
#include "Log/logger.h"

class Globals
{
public:
	IPCsettings* IPCset;
	QCustomPlot* widget1;
	QCustomPlot* widget2;
	Plotter2D* plotter2D;

	Globals()
	{
		IPCset = new IPCsettings(100, 100, 5, 20);
		plotter2D = new Plotter2D();
		logger = new Logger();
	}
private:
	Logger* logger;

};
