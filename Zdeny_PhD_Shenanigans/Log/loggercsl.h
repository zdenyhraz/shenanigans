#pragma once
#include "stdafx.h"
#include "logger.h"

const std::vector<int> LOGLEVEL_CLR{ 10,12,14,11,15 };

struct CslLogger : Logger
{
	CslLogger(LOGLEVEL loglevel) : Logger(loglevel) {};

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	inline void Log(const std::string& msg, LOGLEVEL loglevel) override
	{
		if (loglevel <= m_loglevel)
		{
			SetConsoleTextAttribute(hConsole, LOGLEVEL_CLR[loglevel]);
			cout << "[" + currentTime() + "] [" + LOGLEVEL_STR[loglevel] + "]: " + msg + "\n";
			SetConsoleTextAttribute(hConsole, 7);
		}
	}
};