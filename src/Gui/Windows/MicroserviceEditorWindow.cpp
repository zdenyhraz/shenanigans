#include "MicroserviceEditorWindow.hpp"

void MicroserviceEditorWindow::Initialize()
{
  editor.OnStart();
}

void MicroserviceEditorWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("MicroserviceEditor"))
  {
    ImGui::Separator();
    if (ImGui::Button("Test"))
      LaunchAsync([&]() { Test(); });

    editor.OnFrame();

    ImGui::EndTabItem();
  }
}

void MicroserviceEditorWindow::Test()
{
  editor.workflow.Run();
  LOG_SUCCESS("Workflow test completed");
}
