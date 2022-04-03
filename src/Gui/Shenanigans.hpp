#pragma once
#include "IPCWindow.hpp"

class Shenanigans
{
public:
  static void Run();

private:
  static void Render();
  static void Initialize();
  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
