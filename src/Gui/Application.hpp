#pragma once
#include "Windows/Window.hpp"

class Application
{
  void Render();
  void Initialize();
  void RenderPlotMenu();
  void RenderDemoMenu();
  void RenderThemeMenu();
  static void SetWindowIcon(GLFWwindow* window);
  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

  std::vector<std::unique_ptr<Window>> mWindows;
  bool mShowImGuiDemoWindow = false;
  bool mShowImPlotDemoWindow = false;
  bool mPlotSave = false;

public:
  void Run();
};
