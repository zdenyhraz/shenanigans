#pragma once
#include "Logger.hpp"
#include "TerminalLogger.hpp"

class ImGuiLogger : public Logger
{
  using FallbackLogger = TerminalLogger;
  ImGuiTextBuffer mTextBuffer;
  std::vector<std::pair<int, LogLevel>> mLineOffsets;
  bool mActive = false;
  bool mFallback = true;
  static constexpr usize mMaxMessages = 5000;

public:
  template <typename... Args>
  static void Message(LogLevel logLevel, const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(logLevel, fmt, std::forward<Args>(args)...);
  }

  static void Render() { Get().RenderInternal(); }

  static void SetFallback(bool forward) { Get().mFallback = forward; }

private:
  static ImGuiLogger& Get()
  {
    static ImGuiLogger logger;
    return logger;
  }

  template <typename... Args>
  void LogMessage(LogLevel logLevel, const std::string& fmt, Args&&... args)
  try
  {
    if (not ShouldLog(logLevel)) [[unlikely]]
      return;

    if (mLineOffsets.size() > mMaxMessages)
    {
      Clear();
      Message(Logger::LogLevel::Debug, "Message log cleared after {} messages", mMaxMessages);
    }

    if (mFallback and not mActive) // forward logging to the tfallback logger if this logger is is not being rendered
      FallbackLogger::Message(logLevel, fmt, std::forward<Args>(args)...);

    std::string message = fmt::format("[{}] {}\n", GetCurrentTime(), fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
    i32 oldSize = mTextBuffer.size();
    mTextBuffer.append(message.c_str(), message.c_str() + message.size());
    for (i32 newSize = mTextBuffer.size(); oldSize < newSize; ++oldSize)
      if (mTextBuffer[oldSize] == '\n')
        mLineOffsets.emplace_back(oldSize + 1, logLevel);
  }
  catch (const std::exception& e)
  {
    FallbackLogger::Message(Logger::LogLevel::Error, "LogMessage error: {}", e.what());
  }

  void Clear()
  {
    mTextBuffer.clear();
    mLineOffsets.clear();
    mLineOffsets.emplace_back(0, LogLevel::Trace);
  }

  void RenderInternal();
};
