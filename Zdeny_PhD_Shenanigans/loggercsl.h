#pragma once
#include "stdafx.h"
#include "logger.h"

struct CslLogger : Logger
{
	CslLogger(LOGLEVEL loglevel) : Logger(loglevel) {};

	inline void Log(const std::string& msg, LOGLEVEL loglevel) override
	{
		if (loglevel <= m_loglevel)
		{
			if (loglevel != DEBUG) cout << LOGLEVEL_STRS[loglevel] + "[" + currentTime() + "] [" + LOGLEVEL_STR[loglevel] + "]: " + msg + "\n";
			else cout << LOGLEVEL_STRS[loglevel] + msg + "\n";
		}
	}
};