#include "Shenanigans.hpp"
#include "Gui/Gui.hpp"
#include "Python/Python.hpp"
#include "IPCWindow.hpp"
#include "IPCOptimizeWindow.hpp"
#include "IPCMeasureWindow.hpp"
#include "DiffrotWindow.hpp"

void Shenanigans::Run()
{
  PythonInitialize();
  GLFWInitialize();
  auto window = GLFWCreateWindow(1, 1);
  GLFWInitializeGL(window);
  GLFWSetWindowCallback(window, KeyCallback);
  ImGuiIO& io = ImGuiInitialize(window, 2.5);
  InitializeWindows();

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
  ImGui::Begin("Modules");
  ImGui::Checkbox("IPC parameters", &mWindows.IPCWindow);
  ImGui::Checkbox("IPC optimize", &mWindows.IPCOptimizeWindow);
  ImGui::Checkbox("IPC measure", &mWindows.IPCMeasureWindow);
  ImGui::Checkbox("Diffrot", &mWindows.DiffrotWindow);
  ImGui::End();

  ImGui::ShowDemoWindow();

  if (mWindows.IPCWindow)
    IPCWindow::Render();
  if (mWindows.IPCOptimizeWindow)
    IPCOptimizeWindow::Render();
  if (mWindows.IPCMeasureWindow)
    IPCMeasureWindow::Render();
  if (mWindows.DiffrotWindow)
    DiffrotWindow::Render();
}

void Shenanigans::InitializeWindows()
{
  IPCWindow::Initialize();
  IPCOptimizeWindow::Initialize();
  IPCMeasureWindow::Initialize();
  DiffrotWindow::Initialize();
}

void Shenanigans::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}
