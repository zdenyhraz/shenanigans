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
  const ImVec2 spacing = ImVec2(6, 6);
  const ImVec2 minNodeSize = ImVec2(100, 50);
  const float connectionThickness = 4.0f;
  const float parametersVerticalOffset = 10.0f;
  bool showFlow = false;
  bool showFlowOnce = false;

  void OnStart()
  {
    ed::Config config;
    config.SettingsFile = "MicroserviceEditor.json";
    context = ed::CreateEditor(&config);
    ed::SetCurrentEditor(context);

    auto& editorStyle = ed::GetStyle();
    editorStyle.LinkStrength = 1000;
    editorStyle.FlowMarkerDistance = 30.0f;
    editorStyle.FlowSpeed = 150.0f;
    editorStyle.FlowDuration = 3.0f;
    editorStyle.Colors[ed::StyleColor_Flow] = GetColor(typeid(Microservice::FlowParameter));
    editorStyle.Colors[ed::StyleColor_FlowMarker] = GetColor(typeid(Microservice::FlowParameter));

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

  ImColor GetColor(const std::type_info& type)
  {
    if (type == typeid(Microservice::FlowParameter))
      return ImColor(255, 255, 255);
    if (type == typeid(cv::Mat))
      return ImColor(147, 226, 74);
    if (type == typeid(bool))
      return ImColor(225, 0, 0);

    return ImColor(51, 150, 215);
  };

  void DrawPinIcon(const std::type_info& type, bool connected, int alpha)
  {
    IconType iconType = IconType::Circle;
    ImColor color = GetColor(type);
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

  float GetLongestParameterDisplayLength(Microservice& microservice)
  {
    float maxSize = 0;
    for (const auto& [name, param] : microservice.GetParameters())
      if (param.type == typeid(std::string))
        if (auto textSize = ImGui::CalcTextSize(std::any_cast<std::string>(param.value).c_str()).x; textSize > maxSize)
          maxSize = textSize;

    return maxSize;
  }

  void ShowLabel(const char* label, ImColor color)
  {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
    auto size = ImGui::CalcTextSize(label);

    auto padding = ImGui::GetStyle().FramePadding;
    auto spacing = ImGui::GetStyle().ItemSpacing;

    ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

    auto rectMin = ImGui::GetCursorScreenPos() - padding;
    auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

    auto drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
    ImGui::TextUnformatted(label);
  }

  void ShowFlow() { showFlowOnce = true; }

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
    if (param.type == typeid(float))
      ImGui::InputFloat(fmt::format("{}##{}", param.name, microserviceName).c_str(), std::any_cast<float>(&param.value), 0.01f, 1.0f, "%.3f");
    else if (param.type == typeid(std::string))
      ImGui::InputText(fmt::format("{}##{}", param.name, microserviceName).c_str(), std::any_cast<std::string>(&param.value));
    else if (param.type == typeid(bool))
      ImGui::Checkbox(fmt::format("{}##{}", param.name, microserviceName).c_str(), std::any_cast<bool>(&param.value));
    // else if (std::ranges::contains(param.type.name(), "enum"))
  }

  void RenderNode(Microservice& microservice)
  {
    ed::BeginNode(microservice.GetId());
    auto& style = ImGui::GetStyle();
    auto spacingOrig = style.ItemSpacing;
    style.ItemSpacing = spacing;

    const auto& microserviceName = microservice.GetName();
    const auto inputMaxTextSize = GetLongestInputParameterNameLength(microservice);
    const auto outputMaxTextSize = GetLongestOutputParameterNameLength(microservice);
    const auto inputSize = inputMaxTextSize + pinIconSize;
    const auto outputSizeMin = outputMaxTextSize + pinIconSize;
    const auto ioSizeMin = inputSize + outputSizeMin;
    const auto paramsMaxTextSize = GetLongestParameterNameLength(microservice);
    const auto paramsMaxDisplaySize = GetLongestParameterDisplayLength(microservice);
    const auto paramsSizeMin = microservice.GetParameters().size() ? paramsMaxTextSize + paramsMaxDisplaySize : 0.0f;
    const auto nameSizeMin = ImGui::CalcTextSize(microserviceName.c_str()).x;
    const auto nodeSize = std::max({paramsSizeMin, ioSizeMin, nameSizeMin, minNodeSize.x});
    const auto outputSize = nodeSize - inputSize;

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

    ImGui::Dummy(ImVec2(0, parametersVerticalOffset));
    ImGui::PushItemWidth(paramsMaxDisplaySize);
    for (auto& [name, param] : microservice.GetParameters())
      RenderParameter(param, microserviceName);
    ImGui::PopItemWidth();

    style.ItemSpacing = spacingOrig;
    ed::EndNode();
  }

  void RenderLink(const Microservice::Connection& connection)
  {
    ed::Link(connection.GetId(), connection.inputParameter->GetId(), connection.outputParameter->GetId(), GetColor(connection.outputParameter->type), connectionThickness);

    if (showFlowOnce or showFlow)
      if (connection.outputParameter == &connection.outputMicroservice->GetFlowOutputParameter())
        ed::Flow(connection.GetId(), ed::FlowDirection::Backward);
  }

  Microservice::Connection GetConnection(uintptr_t startId, uintptr_t endId)
  {
    Microservice::Connection connection;
    for (const auto& microservice : workflow.GetMicroservices())
    {
      if (auto param = microservice->FindInputParameter(startId))
      {
        connection.inputMicroservice = microservice.get();
        connection.inputParameter = param.value();
      }
      if (auto param = microservice->FindInputParameter(endId))
      {
        connection.inputMicroservice = microservice.get();
        connection.inputParameter = param.value();
      }
      if (auto param = microservice->FindOutputParameter(startId))
      {
        connection.outputMicroservice = microservice.get();
        connection.outputParameter = param.value();
      }
      if (auto param = microservice->FindOutputParameter(endId))
      {
        connection.outputMicroservice = microservice.get();
        connection.outputParameter = param.value();
      }

      if (connection.outputMicroservice and connection.inputMicroservice)
        break;
    }

    return connection;
  }

  void HandleLinkCreate()
  {
    if (ed::BeginCreate())
    {
      ed::PinId inputPinId, outputPinId;
      if (ed::QueryNewLink(&inputPinId, &outputPinId))
      {
        ImColor failColor(170, 20, 20);
        ImColor successColor(40, 140, 10);
        if (inputPinId and outputPinId)
        {
          auto connection = GetConnection(inputPinId.Get(), outputPinId.Get());
          if (inputPinId == outputPinId)
          {
            ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
          }
          else if (connection.outputMicroservice == connection.inputMicroservice)
          {
            ShowLabel("x Cannot connect to self", failColor);
            ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
          }
          else if (not connection.inputMicroservice)
          {
            ShowLabel("x Cannot connect output to output", failColor);
            ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
          }
          else if (not connection.outputMicroservice)
          {
            ShowLabel("x Cannot connect input to input", failColor);
            ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
          }
          else if (connection.outputParameter->type != connection.inputParameter->type)
          {
            ShowLabel("x Incompatible type", failColor);
            ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
          }
          else
          {
            ShowLabel("+ Create link", successColor);
            if (ed::AcceptNewItem())
              workflow.Connect(std::move(connection));
          }
        }
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
    showFlowOnce = false;

    HandleLinkCreate();
    HandleLinkDelete();

    ed::End();

    if (firstFrame)
      ed::NavigateToContent(0.0f);
    ed::SetCurrentEditor(nullptr);
    firstFrame = false;
  }
};
