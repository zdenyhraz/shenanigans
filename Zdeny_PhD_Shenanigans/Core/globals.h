#pragma once
#include "stdafx.h"
#include "IPC/IPC.h"

class Globals
{
public:
	Globals()
	{
		IPCset = std::make_unique<IPCsettings>( 100, 100, 5, 20 );
	}

	std::unique_ptr<IPCsettings> IPCset;

private:
};
