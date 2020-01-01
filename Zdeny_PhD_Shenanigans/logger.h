#pragma once
#include "stdafx.h"
#include "functionsBaseSTL.h"

 enum LOGLEVEL { SPECIAL, FATAL, EVENT, SUBEVENT, WARN, INFO, DEBUG };
 const std::vector<std::string> LOGLEVEL_STR{ "SPECIAL", "FATAL", "main", "sub", "warning", "info", "debug" };
 const std::vector<std::string> LOGLEVEL_STRS{ "", "", "", "       ", "       ", "       ", "       " };

struct Logger
{
	LOGLEVEL m_loglevel;

	Logger(LOGLEVEL loglevel) : m_loglevel(loglevel) {};

	inline virtual void Log(const std::string& msg, LOGLEVEL loglevel = INFO) = 0;
};