#pragma once
#include <imgui_node_editor.h>
#include "Workflow.hpp"
#include "libs/imgui-nodes/examples/blueprints-example/utilities/drawing.h"
#include "libs/imgui-nodes/examples/blueprints-example/utilities/widgets.h"

namespace ed = ax::NodeEditor;
using ax::Widgets::IconType;

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
    config.SettingsFile = "MicroserviceEditor.json";
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

  ImColor GetIconColor(const std::type_info& type)
  {
    if (type == typeid(MicroserviceFlowParameter))
      return ImColor(255, 255, 255);

    return ImColor(51, 150, 215);
  };

  void DrawPinIcon(const std::type_info& type, bool connected, int alpha)
  {
    IconType iconType = IconType::Circle;
    ImColor color = GetIconColor(type);
    color.Value.w = alpha / 255.0f;

    if (type == typeid(MicroserviceFlowParameter))
      iconType = IconType::Flow;

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

  void RenderInputPin(const MicroserviceInputParameter& param)
  {
    ed::BeginPin(param.GetId(), ed::PinKind::Input);
    ed::PinPivotAlignment(ImVec2(0.0f, 0.5f));
    ed::PinPivotSize(ImVec2(0, 0));
    DrawPinIcon(param.type, false, 255);
    ImGui::SameLine();
    ImGui::TextUnformatted(param.name.c_str());
    ed::EndPin();
  }

  void RenderOutputPin(const MicroserviceOutputParameter& param, float outputColumnTextSize)
  {
    ed::BeginPin(param.GetId(), ed::PinKind::Output);
    ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
    ed::PinPivotSize(ImVec2(0, 0));
    ImGui::Dummy(ImVec2(outputColumnTextSize - ImGui::CalcTextSize(param.name.c_str()).x, 0));
    ImGui::SameLine();
    ImGui::TextUnformatted(param.name.c_str());
    ImGui::SameLine();
    DrawPinIcon(param.type, false, 255);
    ed::EndPin();
  }

  void RenderNodeName(const std::string& name, float nodeSize)
  {
    ImGui::Dummy(ImVec2((nodeSize - ImGui::CalcTextSize(name.c_str()).x) * 0.5, 0));
    ImGui::SameLine();
    ImGui::Text(name.c_str());
  }

  void RenderNode(Microservice& microservice)
  {
    ed::BeginNode(microservice.GetId());
    const auto inputColumnTextSize = GetLongestInputParameterNameLength(microservice);
    const auto outputColumnTextSize = GetLongestOutputParameterNameLength(microservice);
    const auto nodeSize = inputColumnTextSize + outputColumnTextSize + pinIconSize * 2 + ImGui::GetStyle().FramePadding.x * 3;
    RenderNodeName(microservice.GetName(), nodeSize);

    BeginColumn();

    RenderInputPin(microservice.GetStartParameter());
    for (const auto& [name, param] : microservice.GetInputParameters())
      RenderInputPin(param);

    NextColumn();

    RenderOutputPin(microservice.GetFinishParameter(), outputColumnTextSize);
    for (const auto& [name, param] : microservice.GetOutputParameters())
      RenderOutputPin(param, outputColumnTextSize);

    EndColumn();

    ed::EndNode();
  }

  void OnFrame()
  {
    auto& io = ImGui::GetIO();
    ImGui::Separator();
    ed::SetCurrentEditor(context);
    ed::Begin("Microservice Editor", ImVec2(0.0, 0.0f));

    for (const auto& microservice : workflow.GetMicroservices())
      RenderNode(*microservice);

    for (const auto& [microservice, connections] : workflow.GetConnections())
      for (const auto& connection : connections)
        ed::Link(connection.GetId(), connection.inputParameter->GetId(), connection.outputParameter->GetId());

    if (ed::BeginCreate())
    {
      ed::PinId inputPinId, outputPinId;
      if (ed::QueryNewLink(&inputPinId, &outputPinId))
      {
        if (inputPinId and outputPinId)
        {
          if (ed::AcceptNewItem())
            workflow.Connect(inputPinId.Get(), outputPinId.Get());
        }
      }
    }
    ed::EndCreate();

    if (ed::BeginDelete())
    {
      ed::LinkId deletedLinkId;
      while (ed::QueryDeletedLink(&deletedLinkId))
      {
        if (ed::AcceptDeletedItem())
          workflow.Disconnect(deletedLinkId.Get());
      }
    }
    ed::EndDelete();

    ed::End();

    if (firstFrame)
      ed::NavigateToContent(0.0f);
    ed::SetCurrentEditor(nullptr);
    firstFrame = false;
  }
};
