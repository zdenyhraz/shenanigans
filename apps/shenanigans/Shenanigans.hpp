#pragma once

class Shenanigans
{
public:
  static void Run();

private:
  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
