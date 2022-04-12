#pragma once
#include "Logger.hpp"
#include "TerminalLogger.hpp"
#include "Utils/Singleton.hpp"

class ImGuiLogger : public Logger, public Singleton<ImGuiLogger>
{
  using FallbackLogger = TerminalLogger;

  ImGuiTextBuffer mTextBuffer;
  std::vector<std::pair<int, LogLevel>> mLineOffsets;
  bool mActive = false;
  bool mFallback = true;
  static constexpr usize mMaxMessages = 5000;

public:
  void Render();

  void Clear()
  {
    std::scoped_lock lock(mMutex);
    mTextBuffer.clear();
    mLineOffsets.clear();
    mLineOffsets.emplace_back(0, LogLevel::Trace);
  }

  void SetFallback(bool forward) { mFallback = forward; }

  template <typename... Args>
  void Message(LogLevel logLevel, const std::string& fmt, Args&&... args)
  try
  {
    std::scoped_lock lock(mMutex);
    if (not ShouldLog(logLevel))
      [[unlikely]] return;

    if (mLineOffsets.size() > mMaxMessages)
    {
      Clear();
      Message(Logger::LogLevel::Debug, "Message log cleared after {} messages", mMaxMessages);
    }

    if (mFallback and not mActive) // forward logging to the tfallback logger if this logger is is not being rendered
      FallbackLogger::Get().Message(logLevel, fmt, std::forward<Args>(args)...);

    std::string message = fmt::format("[{}] {}\n", GetCurrentTime(), fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
    i32 oldSize = mTextBuffer.size();
    mTextBuffer.append(message.c_str(), message.c_str() + message.size());
    for (i32 newSize = mTextBuffer.size(); oldSize < newSize; ++oldSize)
      if (mTextBuffer[oldSize] == '\n')
        mLineOffsets.emplace_back(oldSize + 1, logLevel);
  }
  catch (const std::exception& e)
  {
    FallbackLogger::Get().Message(Logger::LogLevel::Error, "LogMessage error: {}", e.what());
  }
};
