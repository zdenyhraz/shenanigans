#include "NodeEditorWindow.hpp"
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;
ed::EditorContext* m_Context = nullptr;

void NodeEditorWindow::Initialize()
{
  ed::Config config;
  config.SettingsFile = "Simple.json";
  m_Context = ed::CreateEditor(&config);
}

void NodeEditorWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("NodeEditor"))
  {
    ed::SetCurrentEditor(m_Context);
    ed::Begin("My Editor", ImVec2(0.0, 0.0f));
    int uniqueId = 1;
    // Start drawing nodes.
    ed::BeginNode(uniqueId++);
    ImGui::Text("Node A");
    ed::BeginPin(uniqueId++, ed::PinKind::Input);
    ImGui::Text("-> In");
    ed::EndPin();
    ImGui::SameLine();
    ed::BeginPin(uniqueId++, ed::PinKind::Output);
    ImGui::Text("Out ->");
    ed::EndPin();
    ed::EndNode();
    ed::End();
    ed::SetCurrentEditor(nullptr);

    ImGui::EndTabItem();
  }
}
