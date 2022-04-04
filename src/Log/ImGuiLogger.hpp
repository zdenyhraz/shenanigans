#pragma once
#include "Logger.hpp"
#include "TerminalLogger.hpp"

class ImGuiLogger : public Logger
{
  ImGuiTextBuffer mTextBuffer;
  std::vector<std::pair<int, LogLevel>> mLineOffsets;

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

  static void Render() { Get().RenderInternal(); }

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
    if (not ShouldLog(logLevel))
      [[unlikely]] return;

    if (not glfwInit()) // forward logging to terminal logger if OpenGL is not available
      TerminalLogger::Message(logLevel, fmt, std::forward<Args>(args)...);

    if (mLineOffsets.size() > mMaxMessages)
    {
      Debug("Clearing message log ...");
      Clear();
      Debug("Message log cleared after {} messages", mMaxMessages);
    }

    std::string message = fmt::format("[{}] {}\n", GetCurrentTime(), fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
    i32 oldSize = mTextBuffer.size();
    mTextBuffer.append(message.c_str(), message.c_str() + message.size());
    for (i32 newSize = mTextBuffer.size(); oldSize < newSize; ++oldSize)
      if (mTextBuffer[oldSize] == '\n')
        mLineOffsets.emplace_back(oldSize + 1, logLevel);
  }
  catch (const std::exception& e)
  {
    TerminalLogger::Error("LogMessage error: {}", e.what());
  }

  void Clear()
  {
    mTextBuffer.clear();
    mLineOffsets.clear();
    mLineOffsets.emplace_back(0, LogLevel::Trace);
  }

  void RenderInternal();
};
