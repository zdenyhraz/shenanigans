#pragma once
#include "Logger.hpp"

class ImGuiLogger : public Logger
{
  ImGuiTextBuffer mTextBuffer;
  ImVector<int> mLineOffsets;

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
    int old_size = mTextBuffer.size();
    mTextBuffer.append(message.c_str(), message.c_str() + message.size());
    for (int new_size = mTextBuffer.size(); old_size < new_size; old_size++)
      if (mTextBuffer[old_size] == '\n')
        mLineOffsets.push_back(old_size + 1);
  }

  void Clear()
  {
    mTextBuffer.clear();
    mLineOffsets.clear();
    mLineOffsets.push_back(0);
  }

  void Draw()
  {
    ImGui::Begin("ImGuiLogger");

    if (ImGui::Button("Clear"))
      Clear();

    ImGui::Separator();
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    const char* bufStart = mTextBuffer.begin();
    const char* bufEnd = mTextBuffer.end();

    ImGuiListClipper clipper;
    clipper.Begin(mLineOffsets.Size);
    while (clipper.Step())
    {
      for (int lineNumber = clipper.DisplayStart; lineNumber < clipper.DisplayEnd; lineNumber++)
      {
        const char* lineStart = bufStart + mLineOffsets[lineNumber];
        const char* lineEnd = (lineNumber + 1 < mLineOffsets.Size) ? (bufStart + mLineOffsets[lineNumber + 1] - 1) : bufEnd;
        ImGui::TextUnformatted(lineStart, lineEnd);
      }
    }
    clipper.End();
    ImGui::PopStyleVar();

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
      ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
  }
};
