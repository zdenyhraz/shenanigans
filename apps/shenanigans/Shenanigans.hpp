#pragma once
#include "IPCWindow.hpp"

struct Windows
{
  bool IPCWindow = true;
  bool IPCOptimizeWindow = true;
  bool IPCMeasureWindow = true;
  bool DiffrotWindow = true;
};

class Shenanigans
{
public:
  static void Run();

private:
  inline static Windows mWindows;

  static void Render();
  static void InitializeWindows();
  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
