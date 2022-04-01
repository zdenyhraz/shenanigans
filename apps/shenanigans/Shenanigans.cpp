#include "Shenanigans.hpp"
#include "Gui/Gui.hpp"
#include "Python/Python.hpp"
#include "IPCWindow.hpp"
#include "IPCOptimizeWindow.hpp"

void Shenanigans::Run()
{
  PythonInitialize();
  GLFWInitialize();
  auto window = GLFWCreateWindow(1.5 * 1920, 1.5 * 1080);
  GLFWInitializeGL(window);
  GLFWSetWindowCallback(window, KeyCallback);
  ImGuiIO& io = ImGuiInitialize(window, 2.0);
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
  ImGui::Checkbox("IPC parameters", &mGuiModules.mIPCParameters);
  ImGui::Checkbox("IPC optimize", &mGuiModules.mIPCOptimize);
  ImGui::End();

  if (mGuiModules.mIPCParameters)
    IPCWindow::Render();

  if (mGuiModules.mIPCOptimize)
    IPCOptimizeWindow::Render();
}

void Shenanigans::InitializeWindows()
{
  IPCWindow::Initialize();
  IPCOptimizeWindow::Initialize();
}

void Shenanigans::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}
