#pragma once
#include "Logger.hpp"

class TerminalLogger : public Logger
{
  static const std::array<fmt::text_style, LogLevelCount> GenerateLogLevelColors()
  {
    static constinit auto colors = GetLogLevelColors();
    std::array<fmt::text_style, LogLevelCount> styles{};
    styles[Trace] = fg(colors[Trace]);
    styles[Debug] = fg(colors[Debug]);
    styles[Info] = fg(colors[Info]);
    styles[Warning] = fg(colors[Warning]);
    styles[Error] = fg(colors[Error]);
    return styles;
  }

  template <typename... Args>
  static void MessageInternal(LogLevel logLevel, std::string_view fmt, Args&&... args)
  {
    if (not ShouldLog(logLevel))
      return;

    static const auto logLevelColors = GenerateLogLevelColors();
    fmt::print(logLevelColors[static_cast<size_t>(logLevel)], "[{}] {}\n", GetCurrentTime(), fmt::vformat(fmt, fmt::make_format_args(args...)));
  }

public:
  template <typename... Args>
  static void Message(LogLevel logLevel, std::string_view fmt, Args&&... args)
  {
    Singleton<TerminalLogger>::Get().MessageInternal(logLevel, fmt, std::forward<Args>(args)...);
  }
};
