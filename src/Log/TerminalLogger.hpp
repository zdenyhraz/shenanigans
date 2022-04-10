#pragma once
#include "Logger.hpp"
#include "Utils/Utils.hpp"

class TerminalLogger : public Logger
{
public:
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
    if (not ShouldLog(logLevel)) [[unlikely]]
      return;

    fmt::print("[{}] {}\n", GetCurrentTime(), fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
  }
};
