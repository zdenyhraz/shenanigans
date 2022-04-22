#pragma once

struct ShenanigansParameters
{
  bool plotSave = false;
};

class Shenanigans
{
public:
  static void Run();

private:
  inline static ShenanigansParameters mParameters;

  static void Render();
  static void Initialize();
  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
