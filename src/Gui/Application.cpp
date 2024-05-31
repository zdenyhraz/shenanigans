#include "Application.hpp"
#include "Gui.hpp"
#include "Windows/IPCWindow.hpp"
#include "Windows/AstroWindow.hpp"
#include "Windows/RandomWindow.hpp"
#include "Windows/ObjdetectWindow.hpp"
#include "Windows/MicroserviceEditorWindow.hpp"

void Application::Initialize()
{
  PROFILE_FUNCTION;
  mWindows.push_back(std::make_unique<IPCWindow>());
  mWindows.push_back(std::make_unique<AstroWindow>());
  mWindows.push_back(std::make_unique<RandomWindow>());
  mWindows.push_back(std::make_unique<ObjdetectWindow>());
  mWindows.push_back(std::make_unique<MicroserviceEditorWindow>());

  for (const auto& window : mWindows)
    window->Initialize();
}

void Application::Run()
{
  static constexpr bool hiddenWindow = true;
  static constexpr bool limitFPS = true;
  static constexpr f32 scale = 2.0;
  static constexpr f64 targetFPS = 60;
  static constexpr f64 targetFrametime = 1. / targetFPS;
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
  LOG_SUCCESS("Ready");
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
  if (ImGui::Begin("Shenanigans", nullptr, ImGuiWindowFlags_MenuBar))
  {
    if (ImGui::BeginMenuBar())
    {
      RenderPlotMenu();
      RenderDemoMenu();
      RenderThemeMenu();
      ImGui::EndMenuBar();
    }

    if (ImGui::BeginTabBar("Windows", ImGuiTabBarFlags_AutoSelectNewTabs))
    {
      for (const auto& window : mWindows)
        window->Render();

      ImGui::EndTabBar();
    }
    ImGui::End();
  }

  if (mShowImGuiDemoWindow)
    ImGui::ShowDemoWindow();
  if (mShowImPlotDemoWindow)
    ImPlot::ShowDemoWindow();

  ImGuiPlot::SetSave(mPlotSave);
  PyPlot::SetSave(mPlotSave);
  CvPlot::SetSave(mPlotSave);

  ImGuiLogger::Render();
  ImGuiPlot::Render();
  PyPlot::Render();
  CvPlot::Render();
}
catch (const ShenanigansException& e)
{
  e.Log();
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}
catch (...)
{
  LOG_UNKNOWN_EXCEPTION;
}

void Application::RenderPlotMenu()
{
  if (ImGui::BeginMenu("Plot"))
  {
    if (ImGui::MenuItem("Save plots", nullptr, mPlotSave))
      mPlotSave = !mPlotSave;
    if (ImGui::MenuItem("Clear plots"))
      LaunchAsync([]() { Plot::Clear(); });
    if (ImGui::MenuItem("Debug ImGuiPlots"))
      LaunchAsync([]() { ImGuiPlot::Debug(); });
    if (ImGui::MenuItem("Debug PyPlots"))
      LaunchAsync([]() { PyPlot::Debug(); });
    if (ImGui::MenuItem("Debug CvPlots"))
      LaunchAsync([]() { CvPlot::Debug(); });

    ImGui::EndMenu();
  }
}

void Application::RenderDemoMenu()
{
  if (ImGui::BeginMenu("Demos"))
  {
    if (ImGui::MenuItem("ImGui demo", nullptr, mShowImGuiDemoWindow))
      mShowImGuiDemoWindow = !mShowImGuiDemoWindow;
    if (ImGui::MenuItem("ImPlot demo", nullptr, mShowImPlotDemoWindow))
      mShowImPlotDemoWindow = !mShowImPlotDemoWindow;

    ImGui::EndMenu();
  }
}

void Application::RenderThemeMenu()
{
  if (ImGui::BeginMenu("Theme"))
  {
    if (ImGui::MenuItem("Dark"))
      ImGuiSetDarkTheme();
    if (ImGui::MenuItem("Classic"))
      ImGuiSetClassicTheme();
    if (ImGui::MenuItem("Light"))
      ImGuiSetLightTheme();
    if (ImGui::MenuItem("Hazel"))
      ImGuiSetHazelTheme();

    ImGui::EndMenu();
  }
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
    GLFWCloseWindow(window);
}
