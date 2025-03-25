#ifdef GLAPI

#  include "ImGuiLogger.hpp"

void ImGuiLogger::RenderInternal()
{
  ImGui::Begin("Log");

  if (ImGui::Button("Clear"))
    Clear();
  ImGui::SameLine();
  if (false and ImGui::Button("Debug Log"))
  {
    Message(LogLevel::Trace, "Trace message");
    Message(LogLevel::Debug, "Debug message");
    Message(LogLevel::Info, "Info message");
    Message(LogLevel::Warning, "Warning message");
    Message(LogLevel::Error, "Error message");
  }
  ImGui::SameLine();
  ImGui::Text("%.0f ms/frame (%.0f fps)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::SameLine();
  ImGui::ProgressBar(mProgress, ImVec2(-1, 0), mProgressBarOverlay.c_str());
  ImGui::Separator();
  ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
  const char* bufStart = mTextBuffer.begin();
  const char* bufEnd = mTextBuffer.end();

  ImGuiListClipper clipper;
  clipper.Begin(mLineOffsets.size());
  while (clipper.Step())
  {
    for (int lineNumber = clipper.DisplayStart; lineNumber < clipper.DisplayEnd; lineNumber++)
    {
      const char* lineStart = bufStart + mLineOffsets[lineNumber].first;
      const char* lineEnd = (lineNumber + 1 < static_cast<int>(mLineOffsets.size())) ? (bufStart + mLineOffsets[lineNumber + 1].first - 1) : bufEnd;
      static const auto mLogLevelColors = GenerateLogLevelColors();
      ImGui::PushStyleColor(ImGuiCol_Text, mLogLevelColors[(lineNumber + 1 < static_cast<int>(mLineOffsets.size())) ? static_cast<int>(mLineOffsets[lineNumber + 1].second)
                                                                                                                    : static_cast<int>(Logger::LogLevel::Debug)]);
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

void ImGuiLogger::ClearInternal()
{
  std::scoped_lock lock(mMutex);
  mTextBuffer.clear();
  mLineOffsets.clear();
  mLineOffsets.emplace_back(0, LogLevel::Trace);
}
#endif
