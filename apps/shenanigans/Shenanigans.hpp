#pragma once
#include "IPCWindow.hpp"

struct GuiModules
{
  bool mIPCParameters = true;
  bool mIPCOptimize = true;
};

class Shenanigans
{
public:
  static void Run();

private:
  inline static GuiModules mGuiModules;

  static void Render();
  static void InitializeWindows();
  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
