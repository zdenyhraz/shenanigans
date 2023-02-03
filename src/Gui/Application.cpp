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
  static constexpr f32 scale = 2.0;
  static constexpr f64 targetFPS = 60;
  static constexpr f64 targetFrametime = 1. / targetFPS;
  static constexpr bool limitFPS = true;
  static constexpr bool hiddenWindow = true;
  std::srand(std::time(nullptr));
  ImGuiLogger::SetFallback(false);
  GLFWInitialize();
  auto window = GLFWCreateWindow(1920, 1080, hiddenWindow);
  GLFWInitializeGL(window);
  GLFWSetWindowCallback(window, KeyCallback);
  if constexpr (not hiddenWindow)
    SetWindowIcon(window);
  ImGuiIO& io = ImGuiInitialize(window, scale);
  Initialize();

  LOG_DEBUG("Render loop started");
  f64 lastUpdateTime = 0, elapsedTime = 0;
  while (!glfwWindowShouldClose(window))
  {
    if constexpr (limitFPS)
    {
      if (elapsedTime = glfwGetTime() - lastUpdateTime; elapsedTime < targetFrametime)
        std::this_thread::sleep_for(std::chrono::duration<f64>(targetFrametime - elapsedTime));
      lastUpdateTime += targetFrametime;
    }

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
    if (ImGui::BeginTabBar("Windows", ImGuiTabBarFlags_AutoSelectNewTabs))
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

void Application::SetWindowIcon(GLFWwindow* window)
{
  cv::Mat icon = cv::imread((GetProjectDirectoryPath() / "data/apps/logo.png").string(), cv::IMREAD_COLOR);
  cv::resize(icon, icon, cv::Size(32, 32));
  cv::cvtColor(icon, icon, cv::COLOR_BGR2RGBA);
  cv::normalize(icon, icon, 0, 255, cv::NORM_MINMAX);
  icon.convertTo(icon, CV_8UC4);
  GLFWSetWindowIcon(window, icon.cols, icon.rows, icon.data);
}

void Application::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}
