#pragma once
#include "Logger.hpp"
#include "Utils/Utils.hpp"
#include "Utils/Singleton.hpp"

class TerminalLogger : public Logger, public Singleton<TerminalLogger>
{
public:
  template <typename... Args>
  void Message(LogLevel logLevel, const std::string& fmt, Args&&... args) const
  {
    if (not ShouldLog(logLevel))
      [[unlikely]] return;

    fmt::print("[{}] {}\n", GetCurrentTime(), fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
  }
};
