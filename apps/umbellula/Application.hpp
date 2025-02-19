#pragma once
#include "Window.hpp"

class Application
{
  void Initialize();
  void Render();
  static void RenderThemeMenu();
  static void SetWindowIcon(GLFWwindow* window);
  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

  std::vector<std::unique_ptr<Window>> mWindows;

public:
  void Run();
};
