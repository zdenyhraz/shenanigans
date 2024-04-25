#pragma once
#include "ImGuiLogger.hpp"
#include "LogFunction.hpp"

using MainLogger = ImGuiLogger;

#define SOURCE_FUNCTION std::source_location::current().function_name()
#define SOURCE_FILE std::filesystem::relative(std::filesystem::path(std::source_location::current().file_name()), GetProjectDirectoryPath("src")).string()
#define SOURCE_LINE std::source_location::current().line()
#define SOURCE_LOCATION fmt::format("[{}:{}]", SOURCE_FILE, SOURCE_LINE)

#define LOG_TRACE(...) MainLogger::Message(Logger::LogLevel::Trace, __VA_ARGS__)
#define LOG_DEBUG(...) MainLogger::Message(Logger::LogLevel::Debug, __VA_ARGS__)
#define LOG_INFO(...) MainLogger::Message(Logger::LogLevel::Info, __VA_ARGS__)
#define LOG_SUCCESS(...) MainLogger::Message(Logger::LogLevel::Success, __VA_ARGS__)
#define LOG_WARNING(...) MainLogger::Message(Logger::LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...) MainLogger::Message(Logger::LogLevel::Error, __VA_ARGS__)
#define LOG_EXCEPTION(e) MainLogger::Message(Logger::LogLevel::Error, e.what())
#define LOG_UNKNOWN_EXCEPTION MainLogger::Message(Logger::LogLevel::Error, "Unknown error")
#define LOG_PROGRESS(progress) MainLogger::SetProgress(progress)
#define LOG_PROGRESS_NAME(progressName) MainLogger::SetProgressName(progressName)
#define LOG_PROGRESS_RESET MainLogger::ResetProgress()
#define LOG_VARIABLE(variable) MainLogger::Message(Logger::LogLevel::Info, fmt::format("{}: {} {}", #variable, variable, SOURCE_LOCATION))
#define LOG_FUNCTION LogFunction<MainLogger> logFunction(fmt::format("{} {}", SOURCE_FUNCTION, SOURCE_LOCATION))
#define LOG_FUNCTION_IF(show) LogFunction<MainLogger, show> logFunction(fmt::format("{} {}", SOURCE_FUNCTION, SOURCE_LOCATION))
#define LOG_SCOPE(funName) LogFunction<MainLogger> logScope(funName)
#define LOG_SCOPE_IF(show, funName) LogFunction<MainLogger, show> logScope(funName)

#ifdef __cpp_lib_stacktrace
  #define EXCEPTION(...) ShenanigansException(CreateMessage(__VA_ARGS__), std::source_location::current(), std::stacktrace::current())
  #define STDEXCEPTION(...)                                                                                                                                                        \
    std::runtime_error(                                                                                                                                                            \
        fmt::format("{} error: {} {}\nException stack trace:\n{}", SOURCE_FUNCTION, CreateMessage(__VA_ARGS__), SOURCE_LOCATION, std::to_string(std::stacktrace::current())))
#else
  #define EXCEPTION(...) ShenanigansException(CreateMessage(__VA_ARGS__), std::source_location::current())
  #define STDEXCEPTION(...) std::runtime_error(fmt::format("{} error: {} {}", SOURCE_FUNCTION, CreateMessage(__VA_ARGS__), SOURCE_LOCATION))
#endif
