#include "Shenanigans.hpp"
#include "Gui/Gui.hpp"
#include "Python/Python.hpp"

void Shenanigans::Run()
{
  PythonInitialize();
  GLFWInitialize();
  auto window = GLFWCreateWindow();
  GLFWInitializeGL(window);
  GLFWSetWindowCallback(window, KeyCallback);
  ImGuiIO& io = ImGuiInitialize(window);

  while (!glfwWindowShouldClose(window))
  {
    ImGuiNewFrame();

    {
      ImGui::Begin("ddx");
      ImGui::Button("Hi mom");
      static float value = 0;
      ImGui::DragFloat("value", &value);
      ImGui::End();
    }

    ImGuiRender(window, io);
  }

  ImGuiShutdown();
  GLFWShutdown(window);
}

void Shenanigans::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}
