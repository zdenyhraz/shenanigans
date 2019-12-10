#pragma once
#include "constants.h"
#include "IPC.h"
#include "csllogger.h"
#include "qtlogger.h"

struct Globals
{
	IPCsettings* IPCsettings;
	Logger* Logger;
};
