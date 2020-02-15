#pragma once
#include "stdafx.h"
#include "Core/functionsBaseSTL.h"

 enum LOGLEVEL { SPECIAL, EVENT, FATAL, SUBEVENT, WARN, INFO, DEBUG };
 const std::vector<std::string> LOGLEVEL_STR{ "special", "main", "error", "sub", "warning", "info", "debug" };
 const std::vector<std::string> LOGLEVEL_STRS{ "", "", "       ", "       ", "       ", "       ", "       " };

struct Logger
{
	LOGLEVEL m_loglevel;

	Logger(LOGLEVEL loglevel) : m_loglevel(loglevel) {};

	inline virtual void Log(const std::string& msg, LOGLEVEL loglevel = INFO) = 0;
};