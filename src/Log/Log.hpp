#pragma once
#include "ImGuiLogger.hpp"

using MainLogger = ImGuiLogger;

#define LOG_TRACE(...) MainLogger::Get().Message(Logger::LogLevel::Trace, __VA_ARGS__)
#define LOG_DEBUG(...) MainLogger::Get().Message(Logger::LogLevel::Debug, __VA_ARGS__)
#define LOG_INFO(...) MainLogger::Get().Message(Logger::LogLevel::Info, __VA_ARGS__)
#define LOG_SUCCESS(...) MainLogger::Get().Message(Logger::LogLevel::Success, __VA_ARGS__)
#define LOG_WARNING(...) MainLogger::Get().Message(Logger::LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...) MainLogger::Get().Message(Logger::LogLevel::Error, __VA_ARGS__)
#define LOG_EXCEPTION(e) MainLogger::Get().Message(Logger::LogLevel::Error, "{} error: {}", std::source_location::current().function_name(), e.what())
#define LOG_UNKNOWN_EXCEPTION MainLogger::Get().Message(Logger::LogLevel::Error, "{} error: Unknown error", std::source_location::current().function_name())
