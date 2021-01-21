#pragma once
#include "SpdLogger.h"

#define LOG_TRACE(...) SpdLogger::Get()->trace(__VA_ARGS__)
#define LOG_DEBUG(...) SpdLogger::Get()->debug(__VA_ARGS__)
#define LOG_SUCC(...) SpdLogger::Get()->info(__VA_ARGS__)
#define LOG_INFO(...) SpdLogger::Get()->warn(__VA_ARGS__)
#define LOG_ERROR(...) SpdLogger::Get()->error(__VA_ARGS__)
#define LOG_FATAL(...) SpdLogger::Get()->critical(__VA_ARGS__)
