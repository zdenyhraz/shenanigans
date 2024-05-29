#pragma once
#include "Window.hpp"
#include "NodeEditor.hpp"

class NodeEditorWindow : public Window
{
  void NodeEditorTest();
  void Test();

  NodeEditor nex;

public:
  void Initialize() override;
  void Render() override;
};
