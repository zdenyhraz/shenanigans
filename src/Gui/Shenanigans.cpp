#include "Shenanigans.hpp"
#include "Gui/Gui.hpp"
#include "IPCWindow.hpp"
#include "IPCOptimizeWindow.hpp"
#include "DiffrotWindow.hpp"
#include "SwindWindow.hpp"

void Shenanigans::Run()
{
  ImGuiLogger::SetFallback(false);
  GLFWInitialize();
  auto window = GLFWCreateWindow(1920, 1080, true);
  GLFWInitializeGL(window);
  GLFWSetWindowCallback(window, KeyCallback);
  ImGuiIO& io = ImGuiInitialize(window, 2.0);
  Initialize();
  LOG_DEBUG("Render loop started");

  while (!glfwWindowShouldClose(window))
  {
    ImGuiNewFrame();
    Render();
    ImGuiRender(window, io);
    // PROFILE_FRAME;
  }

  ImGuiShutdown();
  GLFWShutdown(window);
}

void Shenanigans::Render()
{
  IPCWindow::Render();
  IPCOptimizeWindow::Render();
  DiffrotWindow::Render();
  SwindWindow::Render();

  ImGuiLogger::Render();
  ImGuiPlot::Render();
  PyPlot::Render();

  // ImGui::ShowDemoWindow();
  // ImPlot::ShowDemoWindow();
}

void Shenanigans::Initialize()
{
  PROFILE_FUNCTION;
  PyPlot::Initialize();
  IPCWindow::Initialize();
  IPCOptimizeWindow::Initialize();
  DiffrotWindow::Initialize();
  SwindWindow::Initialize();
}

void Shenanigans::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}
