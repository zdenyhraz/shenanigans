#pragma once
#include "TerminalLogger.hpp"
#include "ImGuiLogger.hpp"

using Log = ImGuiLogger;
#define LOG_TRACE(...) Log::Trace(__VA_ARGS__)
#define LOG_DEBUG(...) Log::Debug(__VA_ARGS__)
#define LOG_INFO(...) Log::Info(__VA_ARGS__)
#define LOG_SUCCESS(...) Log::Success(__VA_ARGS__)
#define LOG_WARNING(...) Log::Warning(__VA_ARGS__)
#define LOG_ERROR(...) Log::Error(__VA_ARGS__)

using LogTerminal = TerminalLogger;
#define LOG_TERMINAL_TRACE(...) LogTerminal::Trace(__VA_ARGS__)
#define LOG_TERMINAL_DEBUG(...) LogTerminal::Debug(__VA_ARGS__)
#define LOG_TERMINAL_INFO(...) LogTerminal::Info(__VA_ARGS__)
#define LOG_TERMINAL_SUCCESS(...) LogTerminal::Success(__VA_ARGS__)
#define LOG_TERMINAL_WARNING(...) LogTerminal::Warning(__VA_ARGS__)
#define LOG_TERMINAL_ERROR(...) LogTerminal::Error(__VA_ARGS__)
