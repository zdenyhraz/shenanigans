#pragma once
#include "constants.h"
#include "IPC.h"
#include "loggercsl.h"
#include "loggerqt.h"

struct Globals
{
	IPCsettings* IPCsettings;
	Logger* Logger;
};
