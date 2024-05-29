#include "NodeEditorWindow.hpp"
#include "Microservice/Workflow.hpp"
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;
ed::EditorContext* m_Context = nullptr;

void NodeEditorWindow::Initialize()
{
  ed::Config config;
  config.SettingsFile = "Simple.json";
  m_Context = ed::CreateEditor(&config);
  nex.OnStart();
}

void NodeEditorWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("NodeEditor"))
  {
    ImGui::Separator();
    if (ImGui::Button("Test"))
      LaunchAsync([&]() { Test(); });

    NodeEditorTest();

    ImGui::EndTabItem();
  }
}

void NodeEditorWindow::Test()
{
  Workflow wrk;
  wrk.TestInitialize();
  wrk.Run();
  LOG_SUCCESS("Workflow test completed");
}

void NodeEditorWindow::NodeEditorTest()
{
  nex.OnFrame();
}
