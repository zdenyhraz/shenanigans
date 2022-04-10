#pragma once
#include "ImGuiLogger.hpp"

using Log = ImGuiLogger;
#define LOG_TRACE(...) Log::Message(Logger::LogLevel::Trace, __VA_ARGS__)
#define LOG_DEBUG(...) Log::Message(Logger::LogLevel::Debug, __VA_ARGS__)
#define LOG_INFO(...) Log::Message(Logger::LogLevel::Info, __VA_ARGS__)
#define LOG_SUCCESS(...) Log::Message(Logger::LogLevel::Success, __VA_ARGS__)
#define LOG_WARNING(...) Log::Message(Logger::LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...) Log::Message(Logger::LogLevel::Error, __VA_ARGS__)
#define LOG_EXCEPTION(e) Log::Message(Logger::LogLevel::Error, "{} error: {}", std::source_location::current().function_name(), e.what())
#define LOG_UNKNOWN_EXCEPTION Log::Message(Logger::LogLevel::Error, "{} error: Unknown error", std::source_location::current().function_name())
