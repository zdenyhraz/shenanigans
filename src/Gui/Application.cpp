#include "Application.hpp"
#include "Gui/Gui.hpp"
#include "IPCWindow.hpp"
#include "IPCAppsWindow.hpp"
#include "DiffrotWindow.hpp"
#include "SwindWindow.hpp"
#include "StuffWindow.hpp"
#include "ObjdetectWindow.hpp"
#include "Plot/Plot.hpp"

void Application::Run()
{
  std::srand(std::time(nullptr));
  ImGuiLogger::SetFallback(false);
  GLFWInitialize();
  auto window = GLFWCreateWindow(1920, 1080, true);
  GLFWInitializeGL(window);
  GLFWSetWindowCallback(window, KeyCallback);
  const f32 scale = 2.0;
  ImGuiIO& io = ImGuiInitialize(window, scale);
  Initialize();

  LOG_DEBUG("Render loop started");
  while (!glfwWindowShouldClose(window))
  {
    ImGuiNewFrame();
    Render();
    ImGuiRender(window, io);
    PROFILE_FRAME;
  }

  ImGuiShutdown();
  GLFWShutdown(window);
}

void Application::Render()
try
{
  PROFILE_FUNCTION;
  if (ImGui::Begin("Shenanigans"))
  {
    ImGui::Checkbox("Save plots", &mPlotSave);
    ImGui::Separator();
    if (ImGui::BeginTabBar("Windows"))
    {
      for (const auto& window : mWindows)
        window->Render();

      ImGui::EndTabBar();
    }
    ImGui::End();
  }

  ImGuiLogger::Render();
  ImGuiPlot::Render();
  PyPlot::Render();
  PyPlot::SetSave(mPlotSave);
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}

void Application::Initialize()
{
  PROFILE_FUNCTION;
  mWindows.push_back(std::make_unique<IPCWindow>());
  mWindows.push_back(std::make_unique<IPCAppsWindow>());
  mWindows.push_back(std::make_unique<DiffrotWindow>());
  mWindows.push_back(std::make_unique<SwindWindow>());
  mWindows.push_back(std::make_unique<StuffWindow>());
  mWindows.push_back(std::make_unique<ObjdetectWindow>());

  for (const auto& window : mWindows)
    window->Initialize();
}

void Application::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}
