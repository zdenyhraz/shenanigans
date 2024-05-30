#pragma once
#include <imgui_node_editor.h>
#include "Microservice/Workflow.hpp"
namespace ed = ax::NodeEditor;

struct NodeEditor
{
  ed::EditorContext* m_Context = nullptr; // Editor context, required to trace a editor state.
  bool m_FirstFrame = true;               // Flag set for first frame only, some action need to be executed once.
  int m_NextLinkId = 100;                 // Counter to help generate link ids. In real application this will probably based on pointer to user data structure.
  Workflow m_Workflow;

  void OnStart()
  {
    ed::Config config;
    config.SettingsFile = "BasicInteraction.json";
    m_Context = ed::CreateEditor(&config);
    m_Workflow.TestInitialize();
  }

  void OnStop() { ed::DestroyEditor(m_Context); }

  void ImGuiEx_BeginColumn() { ImGui::BeginGroup(); }

  void ImGuiEx_NextColumn()
  {
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
  }

  void ImGuiEx_EndColumn() { ImGui::EndGroup(); }

  void DrawNode(Microservice& microservice)
  {
    ed::BeginNode(microservice.GetId());
    ImGui::Text(microservice.GetName().c_str());

    ImGuiEx_BeginColumn();
    ed::BeginPin(microservice.GetStartId(), ed::PinKind::Input);
    ImGui::Text("> start");
    ed::EndPin();
    for (const auto& [name, param] : microservice.GetInputParameters())
    {
      ed::BeginPin(param.GetId(), ed::PinKind::Input);
      ImGui::Text(fmt::format("> {}", name).c_str());
      ed::EndPin();
    }

    ImGuiEx_NextColumn();
    ed::BeginPin(microservice.GetFinishId(), ed::PinKind::Input);
    ImGui::Text("finish >");
    ed::EndPin();
    for (const auto& [name, param] : microservice.GetOutputParameters())
    {
      ed::BeginPin(param.GetId(), ed::PinKind::Input);
      ImGui::Text(fmt::format("{} >", name).c_str());
      ed::EndPin();
    }

    ImGuiEx_EndColumn();
    ed::EndNode();
  }

  void OnFrame()
  {
    auto& io = ImGui::GetIO();
    ImGui::Separator();

    ed::SetCurrentEditor(m_Context);

    // Start interaction with editor.
    ed::Begin("Microservice Editor", ImVec2(0.0, 0.0f));

    for (const auto& microservice : m_Workflow.GetMicroservices())
      DrawNode(*microservice);

    for (const auto& connection : m_Workflow.GetMicroserviceConnections())
      ed::Link(connection.GetId(), connection.inputMicroservice->GetStartId(), connection.outputMicroservice->GetFinishId());

    for (const auto& connection : m_Workflow.GetParameterConnections())
      ed::Link(connection.GetId(), connection.inputParameter->GetId(), connection.outputParameter->GetId());

    //
    // 2) Handle interactions
    //

    // Handle creation action, returns true if editor want to create new object (node or link)
    if (ed::BeginCreate())
    {
      ed::PinId inputPinId, outputPinId;
      if (ed::QueryNewLink(&inputPinId, &outputPinId))
      {
        // QueryNewLink returns true if editor want to create new link between pins.
        //
        // Link can be created only for two valid pins, it is up to you to
        // validate if connection make sense. Editor is happy to make any.
        //
        // Link always goes from input to output. User may choose to drag
        // link from output pin or input pin. This determine which pin ids
        // are valid and which are not:
        //   * input valid, output invalid - user started to drag new link from input pin
        //   * input invalid, output valid - user started to drag new link from output pin
        //   * input valid, output valid   - user dragged link over other pin, can be validated

        if (inputPinId && outputPinId) // both are valid, let's accept link
        {
          // ed::AcceptNewItem() return true when user release mouse button.
          if (ed::AcceptNewItem())
          {
            // Since we accepted new link, lets add one to our list of links.
            // m_Links.push_back({ed::LinkId(m_NextLinkId++), inputPinId, outputPinId});

            // TODO: fix this
            // m_Workflow.Connect(*reinterpret_cast<MicroserviceOutputParameter*>(outputPinId), *reinterpret_cast<MicroserviceInputParameter*>(inputPinId));

            // Draw new link.
            // ed::Link(m_Links.back().Id, m_Links.back().InputId, m_Links.back().OutputId);
          }

          // You may choose to reject connection between these nodes
          // by calling ed::RejectNewItem(). This will allow editor to give
          // visual feedback by changing link thickness and color.
        }
      }
    }
    ed::EndCreate(); // Wraps up object creation action handling.

    // Handle deletion action
    if (ed::BeginDelete())
    {
      // There may be many links marked for deletion, let's loop over them.
      ed::LinkId deletedLinkId;
      while (ed::QueryDeletedLink(&deletedLinkId))
      {
        // If you agree that link can be deleted, accept deletion.
        if (ed::AcceptDeletedItem())
        {
          // // Then remove link from your data.
          // for (auto& link : m_Links)
          // {
          //   if (link.Id == deletedLinkId)
          //   {
          //     m_Links.erase(&link);
          //     break;
          //   }
          // }
        }

        // You may reject link deletion by calling:
        // ed::RejectDeletedItem();
      }
    }
    ed::EndDelete(); // Wrap up deletion action

    // End of interaction with editor.
    ed::End();

    if (m_FirstFrame)
      ed::NavigateToContent(0.0f);

    ed::SetCurrentEditor(nullptr);

    m_FirstFrame = false;
  }
};
