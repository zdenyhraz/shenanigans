#pragma once
#include "stdafx.h"
#include "IPC/IPC.h"
#include "Log/logger.h"

class Globals
{
public:
	IPCsettings *IPCset;

	Globals()
	{
		IPCset = new IPCsettings( 100, 100, 5, 20 );
		logger = new Logger();
	}
private:
	Logger *logger;

};
