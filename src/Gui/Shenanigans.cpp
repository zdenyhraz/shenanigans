#include "Shenanigans.hpp"
#include "Gui/Gui.hpp"
#include "IPCWindow.hpp"
#include "IPCOptimizeWindow.hpp"
#include "DiffrotWindow.hpp"
#include "SwindWindow.hpp"
#include "StuffWindow.hpp"

void Shenanigans::Run()
{
  ImGuiLogger::Get().SetFallback(false);
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
  if (ImGui::Begin("Shenanigans"))
  {
    if (ImGui::BeginTabBar("Windows"))
    {
      IPCWindow::Render();
      IPCOptimizeWindow::Render();
      DiffrotWindow::Render();
      SwindWindow::Render();
      StuffWindow::Render();
      ImGui::EndTabBar();
    }
    ImGui::End();
  }

  ImGuiLogger::Get().Render();
  ImGuiPlot::Get().Render();
  PyPlot::Render();

  // ImGui::ShowDemoWindow();
  ImPlot::ShowDemoWindow();
}

void Shenanigans::Initialize()
{
  PROFILE_FUNCTION;
  PyPlot::Initialize();
  IPCWindow::Initialize();
  IPCOptimizeWindow::Initialize();
  DiffrotWindow::Initialize();
  SwindWindow::Initialize();
  StuffWindow::Initialize();
}

void Shenanigans::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}
