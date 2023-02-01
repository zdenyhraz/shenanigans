#pragma once
#include "ImGuiLogger.hpp"

using MainLogger = ImGuiLogger;

#define LOG_TRACE(...) MainLogger::Message(Logger::LogLevel::Trace, __VA_ARGS__)
#define LOG_DEBUG(...) MainLogger::Message(Logger::LogLevel::Debug, __VA_ARGS__)
#define LOG_INFO(...) MainLogger::Message(Logger::LogLevel::Info, __VA_ARGS__)
#define LOG_SUCCESS(...) MainLogger::Message(Logger::LogLevel::Success, __VA_ARGS__)
#define LOG_WARNING(...) MainLogger::Message(Logger::LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...) MainLogger::Message(Logger::LogLevel::Error, __VA_ARGS__)
#define LOG_EXCEPTION(e) MainLogger::Message(Logger::LogLevel::Error, "{} error: {}", std::source_location::current().function_name(), e.what())
#define LOG_UNKNOWN_EXCEPTION MainLogger::Message(Logger::LogLevel::Error, "{} error: Unknown error", std::source_location::current().function_name())
#define LOG_PROGRESS(progress) MainLogger::SetProgress(progress)
#define LOG_PROGRESS_NAME(progressName) MainLogger::SetProgressName(progressName)
#define LOG_PROGRESS_RESET MainLogger::ResetProgress()
