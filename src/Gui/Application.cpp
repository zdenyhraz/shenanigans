#include "Application.hpp"
#include "Gui/Gui.hpp"
#include "Utils/Async.hpp"
#include "Log/ImGuiLogger.hpp"
#include "Plot/ImGuiPlot.hpp"

Application::Application(const std::string& appName_) : appName(appName_)
{
}

void Application::SetIniPath(const std::filesystem::path& iniPath_)
{
  const auto path = GetProjectPath(iniPath_.string());
  if (not std::filesystem::is_regular_file(path))
    return LOG_WARNING(fmt::format("File '{}' does not exist", path));
  iniPath = path;
}

void Application::SetFontPath(const std::filesystem::path& fontPath_)
{
  const auto path = GetProjectPath(fontPath_.string());
  if (not std::filesystem::is_regular_file(path))
    return LOG_WARNING(fmt::format("File '{}' does not exist", path));
  fontPath = path;
}

void Application::SetPlotWindowCount(size_t count)
{
  ImGuiPlot::SetWindowCount(count);
}

void Application::Initialize()
{
  PROFILE_FUNCTION;
  for (auto& window : mWindows)
    window->Initialize();
}

void Application::Run()
{
  static constexpr bool hiddenWindow = true;
  static constexpr bool limitFPS = true;
  static constexpr float scale = 1;
  static constexpr double targetFPS = 60;
  static constexpr double targetFrametime = 1. / targetFPS;
  // flawfinder: ignore
  std::srand(std::time(nullptr));
  ImGuiLogger::SetFallback(false);
  GLFWInitialize();
  auto window = GLFWCreateWindow(1920, 1080, hiddenWindow);
  GLFWInitializeGL(window);
  GLFWSetWindowCallback(window, KeyCallback);
  if constexpr (not hiddenWindow)
    SetWindowIcon(window);
  ImGuiIO& io = ImGuiInitialize(window, scale, iniPath, fontPath);
  Initialize();
  LOG_INFO("Ready");
  double lastUpdateTime = 0, elapsedTime = 0;
  while (!glfwWindowShouldClose(window))
  {
    if constexpr (limitFPS)
    {
      if (elapsedTime = glfwGetTime() - lastUpdateTime; elapsedTime < targetFrametime)
        std::this_thread::sleep_for(std::chrono::duration<double>(targetFrametime - elapsedTime));
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
  if (ImGui::Begin(appName.c_str(), nullptr, ImGuiWindowFlags_MenuBar))
  {
    if (ImGui::BeginMenuBar())
    {
      if constexpr (showThemeMenu)
        RenderThemeMenu();
      if constexpr (showPlotMenu)
        RenderPlotMenu();
      if constexpr (showImGuiDemoMenu)
        RenderDemoMenu();
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

  if (showImGuiDemoWindow)
    ImGui::ShowDemoWindow();
  if (showImPlotDemoWindow)
    ImPlot::ShowDemoWindow();

  ImGuiLogger::Render();
  ImGuiPlot::Render();
}
catch (const ShenanigansException& e)
{
  e.Log();
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}

void Application::RenderPlotMenu()
{
  if (ImGui::BeginMenu("Plot"))
  {
    if (ImGui::MenuItem("Clear plots"))
      LaunchAsync([]() { ImGuiPlot::Clear(); });
    if (ImGui::MenuItem("Debug ImGuiPlots"))
      LaunchAsync([]() { ImGuiPlot::Debug(); });

    ImGui::EndMenu();
  }
}

void Application::RenderDemoMenu()
{
  if (ImGui::BeginMenu("Demos"))
  {
    if (ImGui::MenuItem("ImGui demo", nullptr, showImGuiDemoWindow))
      showImGuiDemoWindow = !showImGuiDemoWindow;
    if (ImGui::MenuItem("ImPlot demo", nullptr, showImPlotDemoWindow))
      showImPlotDemoWindow = !showImPlotDemoWindow;

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
  cv::Mat icon = cv::imread((GetProjectPath() / "data/apps/logo.png").string(), cv::IMREAD_COLOR);
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
