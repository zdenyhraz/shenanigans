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
			std::cout << " [" + currentTime() + "] [" + LOGLEVEL_STR[loglevel] + "]: " + msg + "\n";
		}
	}

	inline void LogValue(const std::string& name, double value) override
	{
		std::cout << " [" + currentTime() + "] [VALUE]: " + name + " = " + to_string(value) + "\n";
	}
};