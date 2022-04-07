#include "ImGuiLogger.hpp"

static consteval ImVec4 GetColorFromU8(f32 R, f32 G, f32 B)
{
  return ImVec4(R / 255, G / 255, B / 255, 1.0f);
}

static consteval std::array<ImVec4, static_cast<i32>(Logger::LogLevel::LogLevelCount)> GenerateLogLevelColors()
{
  std::array<ImVec4, static_cast<i32>(Logger::LogLevel::LogLevelCount)> colors;
  colors[static_cast<usize>(Logger::LogLevel::Trace)] = GetColorFromU8(125, 125, 125);
  colors[static_cast<usize>(Logger::LogLevel::Function)] = GetColorFromU8(125, 125, 125);
  colors[static_cast<usize>(Logger::LogLevel::Debug)] = GetColorFromU8(0, 185, 255);
  colors[static_cast<usize>(Logger::LogLevel::Info)] = GetColorFromU8(235, 235, 235);
  colors[static_cast<usize>(Logger::LogLevel::Success)] = GetColorFromU8(0, 227, 15);
  colors[static_cast<usize>(Logger::LogLevel::Warning)] = GetColorFromU8(255, 200, 0);
  colors[static_cast<usize>(Logger::LogLevel::Error)] = GetColorFromU8(255, 0, 0);
  return colors;
}

void ImGuiLogger::RenderInternal()
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
  ImGui::SameLine();
  ImGui::Text("%.3f ms/frame (%.1f fps)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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
  mActive = true;
}
