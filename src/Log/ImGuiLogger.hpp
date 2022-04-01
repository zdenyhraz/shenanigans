#pragma once
#include "Logger.hpp"

class ImGuiLogger : public Logger
{
  ImGuiTextBuffer mTextBuffer;
  std::vector<std::pair<int, LogLevel>> mLineOffsets;

public:
  template <typename... Args>
  static void Trace(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Trace, fmt, args...);
  }

  template <typename... Args>
  static void Function(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Function, fmt, args...);
  }

  template <typename... Args>
  static void Debug(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Debug, fmt, args...);
  }

  template <typename... Args>
  static void Info(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Info, fmt, args...);
  }

  template <typename... Args>
  static void Success(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Success, fmt, args...);
  }

  template <typename... Args>
  static void Warning(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Warning, fmt, args...);
  }

  template <typename... Args>
  static void Error(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Error, fmt, args...);
  }

  static void Render() { Get().Draw(); }

private:
  static ImGuiLogger& Get()
  {
    static ImGuiLogger logger;
    return logger;
  }

  template <typename... Args>
  void LogMessage(LogLevel logLevel, const std::string& fmt, Args&&... args)
  {
    if (not ShouldLog(logLevel)) [[unlikely]]
      return;

    std::string message = fmt::format("[{}] {}\n", GetCurrentTime(), fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
    i32 oldSize = mTextBuffer.size();
    mTextBuffer.append(message.c_str(), message.c_str() + message.size());
    for (i32 newSize = mTextBuffer.size(); oldSize < newSize; ++oldSize)
      if (mTextBuffer[oldSize] == '\n')
        mLineOffsets.emplace_back(oldSize + 1, logLevel);
  }

  void Clear()
  {
    mTextBuffer.clear();
    mLineOffsets.clear();
    mLineOffsets.emplace_back(0, LogLevel::Trace);
  }

  void Draw();
};
