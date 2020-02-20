#pragma once
#include "stdafx.h"
#include "Core/functionsBaseSTL.h"

enum LOGLEVEL { SUCCESS, FATAL, EVENT, SUBEVENT, DEBUG };
const std::vector<std::string> LOGLEVEL_STR{ "success", "error","main", "sub", "debug" };

struct Logger
{
	LOGLEVEL m_loglevel;

	Logger(LOGLEVEL loglevel) : m_loglevel(loglevel) {};

	inline virtual void Log(const std::string& msg, LOGLEVEL loglevel = DEBUG) = 0;
};