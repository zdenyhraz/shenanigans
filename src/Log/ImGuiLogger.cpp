#include "ImGuiLogger.hpp"

static consteval std::array<ImVec4, static_cast<i32>(Logger::LogLevel::LogLevelCount)> GenerateLogLevelColors()
{
  std::array<ImVec4, static_cast<i32>(Logger::LogLevel::LogLevelCount)> colors;
  colors[static_cast<usize>(Logger::LogLevel::Trace)] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
  colors[static_cast<usize>(Logger::LogLevel::Function)] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
  colors[static_cast<usize>(Logger::LogLevel::Debug)] = ImVec4(0.f / 255, 174.f / 255, 255.0f / 255, 1.0f);
  colors[static_cast<usize>(Logger::LogLevel::Info)] = ImVec4(255.f / 255, 255.f / 255, 0.f / 255, 1.0f);
  colors[static_cast<usize>(Logger::LogLevel::Success)] = ImVec4(102.f / 255, 255.f / 255, 0.f / 255, 1.0f);
  colors[static_cast<usize>(Logger::LogLevel::Warning)] = ImVec4(255.f / 255, 128.f / 255, 0.f / 255, 1.0f);
  colors[static_cast<usize>(Logger::LogLevel::Error)] = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
  return colors;
}

void ImGuiLogger::Draw()
{
  ImGui::Begin("Log");

  if (ImGui::Button("Clear"))
    Clear();
  ImGui::SameLine();
  if (ImGui::Button("Debug"))
  {
    Trace("Trace message");
    Function("Function message");
    Debug("Debug message");
    Info("Info message");
    Success("Success message");
    Warning("Warning message");
    Error("Error message");
  }

  ImGui::Separator();
  ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
  const char* bufStart = mTextBuffer.begin();
  const char* bufEnd = mTextBuffer.end();
  static constexpr std::array<ImVec4, static_cast<i32>(LogLevel::LogLevelCount)> mLogLevelColors = GenerateLogLevelColors();

  ImGuiListClipper clipper;
  clipper.Begin(mLineOffsets.size());
  while (clipper.Step())
  {
    for (i32 lineNumber = clipper.DisplayStart; lineNumber < clipper.DisplayEnd; lineNumber++)
    {
      const char* lineStart = bufStart + mLineOffsets[lineNumber].first;
      const char* lineEnd = (lineNumber + 1 < static_cast<i32>(mLineOffsets.size())) ? (bufStart + mLineOffsets[lineNumber + 1].first - 1) : bufEnd;
      ImGui::PushStyleColor(
          ImGuiCol_Text, mLogLevelColors[(lineNumber + 1 < static_cast<i32>(mLineOffsets.size())) ? static_cast<i32>(mLineOffsets[lineNumber + 1].second) : static_cast<i32>(Logger::LogLevel::Debug)]);
      ImGui::TextUnformatted(lineStart, lineEnd);
      ImGui::PopStyleColor();
    }
  }
  clipper.End();
  ImGui::PopStyleVar();

  if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    ImGui::SetScrollHereY(1.0f);

  ImGui::EndChild();
  ImGui::End();
}
