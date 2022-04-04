#pragma once
#include "Logger.hpp"

class TerminalLogger : public Logger
{
public:
  template <typename... Args>
  static void Trace(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Trace, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void Function(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Function, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void Debug(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Debug, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void Info(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Info, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void Success(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Success, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void Warning(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Warning, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void Error(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Error, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void Message(LogLevel logLevel, const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(logLevel, fmt, std::forward<Args>(args)...);
  }

private:
  static TerminalLogger& Get()
  {
    static TerminalLogger logger;
    return logger;
  }

  template <typename... Args>
  void LogMessage(LogLevel logLevel, const std::string& fmt, Args&&... args)
  {
    if (not ShouldLog(logLevel))
      [[unlikely]] return;

    fmt::print("[{}] {}\n", GetCurrentTime(), fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
  }
};
