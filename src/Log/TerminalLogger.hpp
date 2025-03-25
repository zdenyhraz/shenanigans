#pragma once
#include "Logger.hpp"

class TerminalLogger : public Logger
{
  static const std::array<fmt::text_style, static_cast<int>(Logger::LogLevel::LogLevelCount)> GenerateLogLevelColors()
  {
    static constinit auto colors = GetLogLevelColors();
    std::array<fmt::text_style, static_cast<int>(Logger::LogLevel::LogLevelCount)> styles{};
    styles[static_cast<size_t>(Logger::LogLevel::Trace)] = fg(colors[static_cast<size_t>(Logger::LogLevel::Trace)]);
    styles[static_cast<size_t>(Logger::LogLevel::Function)] = fg(colors[static_cast<size_t>(Logger::LogLevel::Function)]);
    styles[static_cast<size_t>(Logger::LogLevel::Debug)] = fg(colors[static_cast<size_t>(Logger::LogLevel::Debug)]);
    styles[static_cast<size_t>(Logger::LogLevel::Info)] = fg(colors[static_cast<size_t>(Logger::LogLevel::Info)]);
    styles[static_cast<size_t>(Logger::LogLevel::Success)] = fg(colors[static_cast<size_t>(Logger::LogLevel::Success)]);
    styles[static_cast<size_t>(Logger::LogLevel::Warning)] = fg(colors[static_cast<size_t>(Logger::LogLevel::Warning)]);
    styles[static_cast<size_t>(Logger::LogLevel::Error)] = fg(colors[static_cast<size_t>(Logger::LogLevel::Error)]);
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
