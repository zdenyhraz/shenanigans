#pragma once
#include "Window.hpp"

class Application
{
  void Render();
  void Initialize();
  static void SetWindowIcon(GLFWwindow* window);
  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

  std::vector<std::unique_ptr<Window>> mWindows;
  bool mPlotSave = false;

public:
  void Run();
};
