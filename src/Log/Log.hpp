#pragma once
#include "ImGuiLogger.hpp"

using Log = ImGuiLogger;
#define LOG_TRACE(...) Log::Trace(__VA_ARGS__)
#define LOG_DEBUG(...) Log::Debug(__VA_ARGS__)
#define LOG_INFO(...) Log::Info(__VA_ARGS__)
#define LOG_SUCCESS(...) Log::Success(__VA_ARGS__)
#define LOG_WARNING(...) Log::Warning(__VA_ARGS__)
#define LOG_ERROR(...) Log::Error(__VA_ARGS__)
#define LOG_EXCEPTION(e) Log::Error("{} error: {}", std::source_location::current().function_name(), e.what())
