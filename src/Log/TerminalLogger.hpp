#pragma once
#include "Logger.hpp"

class TerminalLogger : public Logger
{
  static consteval std::array<std::string, static_cast<i32>(Logger::LogLevel::LogLevelCount)> GenerateLogLevelNames()
  {
    std::array<std::string, static_cast<i32>(Logger::LogLevel::LogLevelCount)> names;
    names[static_cast<usize>(Logger::LogLevel::Trace)] = "Trace";
    names[static_cast<usize>(Logger::LogLevel::Function)] = "Function";
    names[static_cast<usize>(Logger::LogLevel::Debug)] = "Debug";
    names[static_cast<usize>(Logger::LogLevel::Info)] = "Info";
    names[static_cast<usize>(Logger::LogLevel::Success)] = "Success";
    names[static_cast<usize>(Logger::LogLevel::Warning)] = "Warning";
    names[static_cast<usize>(Logger::LogLevel::Error)] = "Error";
    return names;
  }

  static consteval std::array<std::string, static_cast<i32>(Logger::LogLevel::LogLevelCount)> GenerateLogLevelColors()
  {
    std::array<std::string, static_cast<i32>(Logger::LogLevel::LogLevelCount)> names;
    names[static_cast<usize>(Logger::LogLevel::Trace)] = "\033[0;35m";
    names[static_cast<usize>(Logger::LogLevel::Function)] = "\033[2;35m";
    names[static_cast<usize>(Logger::LogLevel::Debug)] = "\033[1;34m";
    names[static_cast<usize>(Logger::LogLevel::Info)] = "\033[1;32m";
    names[static_cast<usize>(Logger::LogLevel::Success)] = "\033[0;32m";
    names[static_cast<usize>(Logger::LogLevel::Warning)] = "\033[0;33m";
    names[static_cast<usize>(Logger::LogLevel::Error)] = "\033[1;31m";
    return names;
  }

  template <typename... Args>
  void MessageInternal(LogLevel logLevel, std::string_view fmt, Args&&... args) const
  {
    if (not ShouldLog(logLevel))
      return;

    static constinit std::array<std::string, static_cast<i32>(Logger::LogLevel::LogLevelCount)> logLevelColors = GenerateLogLevelColors();
    fmt::print("{}[{}] {}\033[0m\n", logLevelColors[static_cast<usize>(logLevel)], GetCurrentTime(), fmt::vformat(fmt, fmt::make_format_args(args...)));
  }

public:
  template <typename... Args>
  static void Message(LogLevel logLevel, std::string_view fmt, Args&&... args)
  {
    Singleton<TerminalLogger>::Get().MessageInternal(logLevel, fmt, std::forward<Args>(args)...);
  }
};
