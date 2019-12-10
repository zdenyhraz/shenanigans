#pragma once
#include "stdafx.h"
#include "logger.h"

struct CslLogger : Logger
{
	CslLogger(LOGLEVEL loglevel) : Logger(loglevel) {};

	inline void LogMessage(const std::string& msg, LOGLEVEL loglevel) override
	{
		if (loglevel <= m_loglevel)
		{
			std::cout << LOGLEVEL_STR2[loglevel] + "[" + currentTime() + "] [" + LOGLEVEL_STR[loglevel] + "]: " + msg + "\n";
		}
	}
};