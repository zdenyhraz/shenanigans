#pragma once
#include "stdafx.h"
#include "functionsBaseSTL.h"

enum LOGLEVEL { FATAL, WARN, SPECIAL, INFO, DEBUG };
const std::vector<std::string> LOGLEVEL_STR{ "FATAL", "WARN", "SPECIAL", "INFO", "DEBUG" };

struct Logger
{
	LOGLEVEL m_loglevel;

	Logger(LOGLEVEL loglevel) : m_loglevel(loglevel) {};

	inline virtual void LogMessage(const std::string& msg, LOGLEVEL loglevel) = 0;
	
	inline virtual void LogValue(const std::string& name, double value) = 0;
};