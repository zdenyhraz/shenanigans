#pragma once
#include "Gui/Window.hpp"
#include "Microservice/MicroserviceEditor.hpp"

class MicroserviceEditorWindow : public Window
{
  void TestInitialize();
  void TestInitializeCX();
  void ShowFlow();
  void Run();
  void RunRepeat();

  MicroserviceEditor editor;

public:
  void Initialize() override;
  void Render() override;
};
