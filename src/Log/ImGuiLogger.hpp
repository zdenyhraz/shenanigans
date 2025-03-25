#pragma once
#ifdef GLAPI
#  include "Logger.hpp"
#  include "TerminalLogger.hpp"

class ImGuiLogger : public Logger
{
  using FallbackLogger = TerminalLogger;

  static const ImVec4 GetImColor(const fmt::rgb& color)
  {
    return ImVec4(static_cast<float>(color.r) / 255, static_cast<float>(color.g) / 255, static_cast<float>(color.b) / 255, 1.0f);
  }

  static const std::array<ImVec4, LogLevelCount> GenerateLogLevelColors()
  {
    static constinit auto colors = GetLogLevelColors();
    std::array<ImVec4, LogLevelCount> imcolors{};
    imcolors[Trace] = GetImColor(colors[Trace]);
    imcolors[Debug] = GetImColor(colors[Debug]);
    imcolors[Info] = GetImColor(colors[Info]);
    imcolors[Warning] = GetImColor(colors[Warning]);
    imcolors[Error] = GetImColor(colors[Error]);
    return imcolors;
  }

  ImGuiTextBuffer mTextBuffer;
  std::vector<std::pair<int, LogLevel>> mLineOffsets{{0, LogLevel::Trace}};
  bool mActive = false;
  bool mFallback = false;
  static constexpr size_t mMaxMessages = 10000;

  void RenderInternal();
  void ClearInternal();
  void SetFallbackInternal(bool fallback) { mFallback = fallback; }

  template <typename... Args>
  void MessageInternal(LogLevel logLevel, std::string_view fmt, Args&&... args)
  try
  {
    std::scoped_lock lock(mMutex);
    if (not ShouldLog(logLevel))
      return;

    if (mFallback and not mActive) // forward logging to the fallback logger if this logger is is not being rendered
      return FallbackLogger::Message(logLevel, fmt, std::forward<Args>(args)...);

    if (mLineOffsets.size() > mMaxMessages)
    {
      Clear();
      Message(Logger::LogLevel::Debug, "Message log cleared after {} messages", mMaxMessages);
    }

    std::string message = fmt::format("[{}] {}\n", GetCurrentTime(), fmt::vformat(fmt, fmt::make_format_args(args...)));
    int oldSize = mTextBuffer.size();
    mTextBuffer.append(message.c_str(), message.c_str() + message.size());
    for (int newSize = mTextBuffer.size(); oldSize < newSize; ++oldSize)
      if (mTextBuffer[oldSize] == '\n')
        mLineOffsets.emplace_back(oldSize + 1, logLevel);
  }
  catch (const std::exception& e)
  {
    FallbackLogger::Message(Logger::LogLevel::Error, "LogMessage error: {}", e.what());
  }

public:
  static void Render()
  {
    PROFILE_FUNCTION;
    Singleton<ImGuiLogger>::Get().RenderInternal();
  }

  static void Clear() { Singleton<ImGuiLogger>::Get().ClearInternal(); }
  static void SetFallback(bool forward) { Singleton<ImGuiLogger>::Get().SetFallbackInternal(forward); }
  template <typename... Args>
  static void Message(LogLevel logLevel, std::string_view fmt, Args&&... args)
  {
    Singleton<ImGuiLogger>::Get().MessageInternal(logLevel, fmt, std::forward<Args>(args)...);
  }
};
#endif
