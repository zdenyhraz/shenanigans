#pragma once
#include "Utils/Formatters.hpp"
#include "Utils/Exception.hpp"
#include "Logger.hpp"
#include "LogFunction.hpp"

#ifdef GLAPI
#  include "ImGuiLogger.hpp"
using MainLogger = ImGuiLogger;
#else
#  include "TerminalLogger.hpp"
using MainLogger = TerminalLogger;
#endif

// logging
#define LOG_TRACE(...) MainLogger::Message(Logger::LogLevel::Trace, __VA_ARGS__)
#define LOG_DEBUG(...) MainLogger::Message(Logger::LogLevel::Debug, __VA_ARGS__)
#define LOG_INFO(...) MainLogger::Message(Logger::LogLevel::Info, __VA_ARGS__)
#define LOG_WARNING(...) MainLogger::Message(Logger::LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...) MainLogger::Message(Logger::LogLevel::Error, __VA_ARGS__)

// exception logging
#define EXCEPTION(...) ShenanigansException(CreateMessage(__VA_ARGS__), std::source_location::current())
#define LOG_EXCEPTION(e) LOG_ERROR("Error: {}", e.what())
#define LOG_UNKNOWN_EXCEPTION LOG_ERROR("Unknown error")

// progress logging
#define LOG_PROGRESS(progress) MainLogger::SetProgress(progress)
#define LOG_PROGRESS_NAME(progressName) MainLogger::SetProgressName(progressName)
#define LOG_PROGRESS_RESET MainLogger::ResetProgress()

// source location
#define SOURCE_FUNCTION std::source_location::current().function_name()
#define SOURCE_FILE std::filesystem::relative(std::filesystem::path(std::source_location::current().file_name()), GetProjectPath("src")).string()
#define SOURCE_LINE std::source_location::current().line()
#define SOURCE_LOCATION fmt::format("[{}:{}]", SOURCE_FILE, SOURCE_LINE)

// variable and function logging
#define LOG_VARIABLE(variable) LOG_INFO("{}: {} {}", #variable, variable, SOURCE_LOCATION)
#define LOG_FUNCTION LogFunction<MainLogger> logFunction(fmt::format("{} {}", SOURCE_FUNCTION, SOURCE_LOCATION))
#define LOG_FUNCTION_IF(show) LogFunction<MainLogger, show> logFunction(fmt::format("{} {}", SOURCE_FUNCTION, SOURCE_LOCATION))
#define LOG_SCOPE(funName) LogFunction<MainLogger> logScope(funName)
#define LOG_SCOPE_IF(show, funName) LogFunction<MainLogger, show> logScope(funName)
