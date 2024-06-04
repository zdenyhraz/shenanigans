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
  const int paramSize = 200;

  void OnStart()
  {
    ed::Config config;
    config.SettingsFile = "MicroserviceEditor.json";
    context = ed::CreateEditor(&config);
    ed::SetCurrentEditor(context);

    auto& editorStyle = ed::GetStyle();
    editorStyle.LinkStrength = 1000;

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
    if (type == typeid(Microservice::FlowParameter))
      return ImColor(255, 255, 255);

    return ImColor(51, 150, 215);
  };

  void DrawPinIcon(const std::type_info& type, bool connected, int alpha)
  {
    IconType iconType = IconType::Circle;
    ImColor color = GetIconColor(type);
    color.Value.w = alpha / 255.0f;

    if (type == typeid(Microservice::FlowParameter))
      iconType = IconType::Flow;

    ax::Widgets::Icon(ImVec2(pinIconSize, pinIconSize), iconType, connected, color, ImColor(32, 32, 32, alpha));
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

  float GetLongestParameterNameLength(Microservice& microservice)
  {
    float maxSize = 0;
    for (const auto& [name, param] : microservice.GetParameters())
      if (auto textSize = ImGui::CalcTextSize(name.c_str()).x; textSize > maxSize)
        maxSize = textSize;

    return maxSize;
  }

  void RenderInputPin(const Microservice::InputParameter& param, bool connected)
  {
    ed::BeginPin(param.GetId(), ed::PinKind::Input);
    ed::PinPivotAlignment(ImVec2(0.0f, 0.5f));
    ed::PinPivotSize(ImVec2(0, 0));
    DrawPinIcon(param.type, connected, 255);
    ImGui::SameLine();
    ImGui::TextUnformatted(param.name.c_str());
    ed::EndPin();
  }

  void RenderOutputPin(const Microservice::OutputParameter& param, bool connected, float outputSize)
  {
    ImGui::Dummy(ImVec2(outputSize - ImGui::CalcTextSize(param.name.c_str()).x - pinIconSize, 0));
    ImGui::SameLine();
    ed::BeginPin(param.GetId(), ed::PinKind::Output);
    ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
    ed::PinPivotSize(ImVec2(0, 0));
    ImGui::TextUnformatted(param.name.c_str());
    ImGui::SameLine();
    DrawPinIcon(param.type, connected, 255);
    ed::EndPin();
  }

  void RenderNodeName(const std::string& name, float nodeSize)
  {
    ImGui::Dummy(ImVec2((nodeSize - ImGui::CalcTextSize(name.c_str()).x) * 0.5, 0));
    ImGui::SameLine();
    ImGui::Text(name.c_str());
  }

  void RenderParameter(Microservice::Parameter& param, const std::string& microserviceName)
  {
    ImGui::PushItemWidth(paramSize);

    if (param.type == typeid(float))
      ImGui::InputFloat(fmt::format("{}##{}", param.name, microserviceName).c_str(), std::any_cast<float>(&param.value), 0.01f, 1.0f, "%.3f");
    else if (param.type == typeid(std::string))
      ImGui::InputText(fmt::format("{}##{}", param.name, microserviceName).c_str(), std::any_cast<std::string>(&param.value));

    ImGui::PopItemWidth();
  }

  void RenderNode(Microservice& microservice)
  {
    ed::BeginNode(microservice.GetId());
    const auto inputMaxTextSize = GetLongestInputParameterNameLength(microservice);
    const auto outputMaxTextSize = GetLongestOutputParameterNameLength(microservice);
    const auto inputSize = inputMaxTextSize + pinIconSize;
    const auto outputSizeMin = outputMaxTextSize + pinIconSize;
    const auto ioSizeMin = inputSize + outputSizeMin;

    const auto paramsMaxTextSize = GetLongestParameterNameLength(microservice);
    const auto paramsSizeMin = microservice.GetParameters().size() ? paramsMaxTextSize + paramSize : 0.0f;

    const auto nodeSize = std::max(paramsSizeMin, ioSizeMin);
    const auto outputSize = nodeSize - inputSize;

    const auto& microserviceName = microservice.GetName();

    if (false and firstFrame)
    {
      LOG_DEBUG("Microservice '{}' gui parameters:", microserviceName);
      LOG_DEBUG("   ioSizeMin: {}", ioSizeMin);
      LOG_DEBUG("   paramsMaxTextSize: {}", paramsMaxTextSize);
      LOG_DEBUG("   paramsSizeMin: {}", paramsSizeMin);
      LOG_DEBUG("   nodeSize: {}", nodeSize);
      LOG_DEBUG("   inputSize: {}", inputSize);
      LOG_DEBUG("   outputSize: {}", outputSize);
    }

    RenderNodeName(microserviceName, nodeSize);

    BeginColumn();

    RenderInputPin(microservice.GetFlowInputParameter(), microservice.IsInputConnected(microservice.GetFlowInputParameter()));
    for (const auto& [name, param] : microservice.GetInputParameters())
      RenderInputPin(param, microservice.IsInputConnected(param));

    NextColumn();

    RenderOutputPin(microservice.GetFlowOutputParameter(), microservice.IsOutputConnected(microservice.GetFlowOutputParameter()), outputSize);
    for (const auto& [name, param] : microservice.GetOutputParameters())
      RenderOutputPin(param, microservice.IsOutputConnected(param), outputSize);

    EndColumn();

    for (auto& [name, param] : microservice.GetParameters())
      RenderParameter(param, microserviceName);

    ed::EndNode();
  }

  void RenderLink(const Microservice::Connection& connection)
  {
    ed::Link(connection.GetId(), connection.inputParameter->GetId(), connection.outputParameter->GetId());
    if constexpr (false)
      if (connection.outputParameter == &connection.outputMicroservice->GetFlowOutputParameter())
        ed::Flow(connection.GetId(), ed::FlowDirection::Backward);
  }

  void HandleLinkCreate()
  {
    if (ed::BeginCreate())
    {
      ed::PinId inputPinId, outputPinId;
      if (ed::QueryNewLink(&inputPinId, &outputPinId))
      {
        if (inputPinId and outputPinId)
          if (ed::AcceptNewItem())
            workflow.Connect(inputPinId.Get(), outputPinId.Get());
      }
    }
    ed::EndCreate();
  }

  void HandleLinkDelete()
  {
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
  }

  void OnFrame()
  {
    auto& io = ImGui::GetIO();
    ImGui::Separator();
    ed::SetCurrentEditor(context);
    ed::Begin("Microservice Editor", ImVec2(0.0, 0.0f));

    for (const auto& microservice : workflow.GetMicroservices())
      RenderNode(*microservice);

    for (const auto& connection : workflow.GetConnections())
      RenderLink(connection);

    HandleLinkCreate();
    HandleLinkDelete();

    ed::End();

    if (firstFrame)
      ed::NavigateToContent(0.0f);
    ed::SetCurrentEditor(nullptr);
    firstFrame = false;
  }
};
