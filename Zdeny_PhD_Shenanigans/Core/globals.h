#pragma once
#include "stdafx.h"
#include "IPC/IPC.h"
#include "Plot/plotterqt2D.h"
#include "Log/logger.h"

class Globals
{
public:
	IPCsettings *IPCset;
	QCustomPlot *widget1;
	QCustomPlot *widget2;
	Plot2D *plot2D;

	Globals()
	{
		IPCset = new IPCsettings( 100, 100, 5, 20 );
		plot2D = new Plot2D();
		logger = new Logger();
	}
private:
	Logger *logger;

};
