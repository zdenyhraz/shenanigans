#pragma once
#include "Window.hpp"
#include "Microservice/MicroserviceEditor.hpp"

class MicroserviceEditorWindow : public Window
{
  void TestInitialize();
  void Run();

  MicroserviceEditor editor;

public:
  void Initialize() override;
  void Render() override;
};
