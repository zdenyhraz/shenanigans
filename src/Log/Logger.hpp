#pragma once
#include "QtLogger.hpp"

using Logger = QtLogger;

#define LOG_TRACE(...) Logger::Trace(__VA_ARGS__)
#define LOG_DEBUG(...) Logger::Debug(__VA_ARGS__)
#define LOG_INFO(...) Logger::Info(__VA_ARGS__)
#define LOG_SUCCESS(...) Logger::Success(__VA_ARGS__)
#define LOG_WARNING(...) Logger::Warning(__VA_ARGS__)
#define LOG_ERROR(...) Logger::Error(__VA_ARGS__)
