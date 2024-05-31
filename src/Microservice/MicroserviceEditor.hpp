#pragma once
#include <imgui_node_editor.h>
#include "Workflow.hpp"
#include "libs/imgui-nodes/examples/blueprints-example/utilities/drawing.h"
#include "libs/imgui-nodes/examples/blueprints-example/utilities/widgets.h"

namespace ed = ax::NodeEditor;
using ax::Widgets::IconType;

enum class PinType
{
  Flow,
  Bool,
  Int,
  Float,
  String,
  Object,
  Function,
  Delegate,
};

struct MicroserviceEditor
{
  ed::EditorContext* context = nullptr;
  bool firstFrame = true;
  Workflow workflow;
  const int pinIconSize = 24;
  // const int nodeWidth = 200;

  void OnStart()
  {
    ed::Config config;
    config.SettingsFile = "BasicInteraction.json";
    context = ed::CreateEditor(&config);
    ed::SetCurrentEditor(context);

    workflow.TestInitialize();

    int ypos = 0;
    int xpos = 0;
    int step = 150;
    for (const auto& microservice : workflow.GetMicroservices())
      ed::SetNodePosition(microservice->GetId(), ImVec2(xpos += step, ypos += step));
  }

  void OnStop() { ed::DestroyEditor(context); }

  void BeginColumn() { ImGui::BeginGroup(); }

  void NextColumn()
  {
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
  }

  void EndColumn() { ImGui::EndGroup(); }

  ImColor GetIconColor(PinType type)
  {
    switch (type)
    {
    default:
    case PinType::Flow:
      return ImColor(255, 255, 255);
    case PinType::Bool:
      return ImColor(220, 48, 48);
    case PinType::Int:
      return ImColor(68, 201, 156);
    case PinType::Float:
      return ImColor(147, 226, 74);
    case PinType::String:
      return ImColor(124, 21, 153);
    case PinType::Object:
      return ImColor(51, 150, 215);
    case PinType::Function:
      return ImColor(218, 0, 183);
    case PinType::Delegate:
      return ImColor(255, 48, 48);
    }
  };

  void DrawPinIcon(PinType type, bool connected, int alpha)
  {
    IconType iconType;
    ImColor color = GetIconColor(type);
    color.Value.w = alpha / 255.0f;
    switch (type)
    {
    case PinType::Flow:
      iconType = IconType::Flow;
      break;
    case PinType::Bool:
      iconType = IconType::Circle;
      break;
    case PinType::Int:
      iconType = IconType::Circle;
      break;
    case PinType::Float:
      iconType = IconType::Circle;
      break;
    case PinType::String:
      iconType = IconType::Circle;
      break;
    case PinType::Object:
      iconType = IconType::Circle;
      break;
    case PinType::Function:
      iconType = IconType::Circle;
      break;
    case PinType::Delegate:
      iconType = IconType::Square;
      break;
    default:
      return;
    }

    ax::Widgets::Icon(ImVec2(static_cast<float>(pinIconSize), static_cast<float>(pinIconSize)), iconType, connected, color, ImColor(32, 32, 32, alpha));
  };

  float GetLongestOutputParameterNameLength(Microservice& microservice)
  {
    float maxSize = 0;
    for (const auto& [name, param] : microservice.GetOutputParameters())
      if (auto textSize = ImGui::CalcTextSize(name.c_str()).x; textSize > maxSize)
        maxSize = textSize;

    return maxSize;
  }

  float GetLongestInputParameterNameLength(Microservice& microservice)
  {
    float maxSize = 0;
    for (const auto& [name, param] : microservice.GetInputParameters())
      if (auto textSize = ImGui::CalcTextSize(name.c_str()).x; textSize > maxSize)
        maxSize = textSize;

    return maxSize;
  }

  void RenderInputPin(const std::string& name, const MicroserviceInputParameter& param)
  {
    ed::BeginPin(param.GetId(), ed::PinKind::Input);
    ed::PinPivotAlignment(ImVec2(0.0f, 0.5f));
    ed::PinPivotSize(ImVec2(0, 0));
    DrawPinIcon(PinType::Object, false, 255);
    ImGui::SameLine();
    ImGui::TextUnformatted(name.c_str());
    ed::EndPin();
  }

  void RenderOutputPin(const std::string& name, const MicroserviceOutputParameter& param, float outputColumnTextSize)
  {
    ed::BeginPin(param.GetId(), ed::PinKind::Output);
    ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
    ed::PinPivotSize(ImVec2(0, 0));
    ImGui::Dummy(ImVec2(outputColumnTextSize - ImGui::CalcTextSize(name.c_str()).x, 0));
    ImGui::SameLine();
    ImGui::TextUnformatted(name.c_str());
    ImGui::SameLine();
    DrawPinIcon(PinType::Object, false, 255);
    ed::EndPin();
  }

  void RenderInputFlowPin(Microservice& microservice)
  {
    ed::BeginPin(microservice.GetStartId(), ed::PinKind::Input);
    ed::PinPivotAlignment(ImVec2(0.0f, 0.5f));
    ed::PinPivotSize(ImVec2(0, 0));
    DrawPinIcon(PinType::Flow, false, 255); // TODO: check if connected
    ed::EndPin();
  }

  void RenderOutputFlowPin(Microservice& microservice, float outputColumnTextSize)
  {
    ed::BeginPin(microservice.GetFinishId(), ed::PinKind::Output);
    ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
    ed::PinPivotSize(ImVec2(0, 0));
    ImGui::Dummy(ImVec2(outputColumnTextSize + ImGui::GetStyle().FramePadding.x * 2, 0));
    ImGui::SameLine();
    DrawPinIcon(PinType::Flow, false, 255); // TODO: check if connected
    ed::EndPin();
  }

  void RenderNodeName(const std::string& name, float nodeSize)
  {
    ImGui::Dummy(ImVec2((nodeSize - ImGui::CalcTextSize(name.c_str()).x) * 0.5, 0));
    ImGui::SameLine();
    ImGui::Text(name.c_str());
  }

  void RenderNode(Microservice& microservice, WorkflowType workflowType)
  {
    ed::BeginNode(microservice.GetId());
    const auto inputColumnTextSize = GetLongestInputParameterNameLength(microservice);
    const auto outputColumnTextSize = GetLongestOutputParameterNameLength(microservice);
    const auto nodeSize = inputColumnTextSize + outputColumnTextSize + pinIconSize * 2 + ImGui::GetStyle().FramePadding.x * 3;
    RenderNodeName(microservice.GetName(), nodeSize);

    BeginColumn();

    if (workflowType != WorkflowType::Simple)
      RenderInputFlowPin(microservice);
    for (const auto& [name, param] : microservice.GetInputParameters())
      RenderInputPin(name, param);

    NextColumn();

    if (workflowType != WorkflowType::Simple)
      RenderOutputFlowPin(microservice, outputColumnTextSize);
    for (const auto& [name, param] : microservice.GetOutputParameters())
      RenderOutputPin(name, param, outputColumnTextSize);

    EndColumn();

    ed::EndNode();
  }

  void OnFrame()
  {
    auto& io = ImGui::GetIO();
    ImGui::Separator();

    ed::SetCurrentEditor(context);

    // Start interaction with editor.
    ed::Begin("Microservice Editor", ImVec2(0.0, 0.0f));

    for (const auto& microservice : workflow.GetMicroservices())
      RenderNode(*microservice, workflow.GetType());

    for (const auto& [microservice, connections] : workflow.GetConnections())
      for (const auto& connection : connections)
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

        if (inputPinId and outputPinId) // both are valid, let's accept link
        {
          // ed::AcceptNewItem() return true when user release mouse button.
          if (ed::AcceptNewItem())
            workflow.Connect(inputPinId.Get(), outputPinId.Get());

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
          workflow.Disconnect(deletedLinkId.Get());

        // You may reject link deletion by calling:
        // ed::RejectDeletedItem();
      }
    }
    ed::EndDelete(); // Wrap up deletion action

    // End of interaction with editor.
    ed::End();

    if (firstFrame)
      ed::NavigateToContent(0.0f);

    ed::SetCurrentEditor(nullptr);

    firstFrame = false;
  }
};
