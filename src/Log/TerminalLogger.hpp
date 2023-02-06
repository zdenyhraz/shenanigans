#pragma once
#include "Logger.hpp"
#include "Utils/Singleton.hpp"
#include "Utils/DateTime.hpp"

class TerminalLogger : public Logger
{
  template <typename... Args>
  void MessageInternal(LogLevel logLevel, const std::string& fmt, Args&&... args) const
  {
    if (not ShouldLog(logLevel)) [[unlikely]]
      return;

    fmt::print("[{}] {}\n", GetCurrentTime(), fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
  }

public:
  template <typename... Args>
  static void Message(LogLevel logLevel, const std::string& fmt, Args&&... args)
  {
    Singleton<TerminalLogger>::Get().MessageInternal(logLevel, fmt, std::forward<Args>(args)...);
  }
};
