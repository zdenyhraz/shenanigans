#include "Shenanigans.hpp"
#include "Gui/Gui.hpp"
#include "IPCWindow.hpp"
#include "IPCOptimizeWindow.hpp"
#include "DiffrotWindow.hpp"

void Shenanigans::Run()
{
  GLFWInitialize();
  auto window = GLFWCreateWindow(1, 1);
  GLFWInitializeGL(window);
  GLFWSetWindowCallback(window, KeyCallback);
  ImGuiIO& io = ImGuiInitialize(window, 2.0);
  Initialize();

  while (!glfwWindowShouldClose(window))
  {
    ImGuiNewFrame();
    Render();
    ImGuiRender(window, io);
  }

  ImGuiShutdown();
  GLFWShutdown(window);
}

void Shenanigans::Render()
{
  PyPlot::Render();
  ImGuiLogger::Render();
  // ImGui::ShowDemoWindow();
  IPCWindow::Render();
  IPCOptimizeWindow::Render();
  DiffrotWindow::Render();
}

void Shenanigans::Initialize()
{
  PyPlot::Initialize();
  IPCWindow::Initialize();
  IPCOptimizeWindow::Initialize();
  DiffrotWindow::Initialize();
}

void Shenanigans::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}
