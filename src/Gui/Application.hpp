#pragma once
#include "Gui/Window.hpp"

class Application
{
  void Initialize();
  void Render();
  static void RenderPlotMenu();
  void RenderDemoMenu();
  static void RenderThemeMenu();
  static void SetWindowIcon(GLFWwindow* window);
  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

  std::vector<std::unique_ptr<Window>> mWindows;
  std::string appName;
  std::filesystem::path iniPath = "imgui.ini";
  std::filesystem::path fontPath = "CascadiaCode.ttf";
  bool showImGuiDemoWindow = false;
  bool showImPlotDemoWindow = false;
  static constexpr bool showPlotMenu = true;
  static constexpr bool showImGuiDemoMenu = true;
  static constexpr bool showThemeMenu = true;

public:
  Application(const std::string& appName_);
  void SetIniPath(const std::filesystem::path& iniPath_);
  void SetFontPath(const std::filesystem::path& fontPath_);
  static void SetPlotWindowCount(size_t count);
  void Run();

  template <typename T>
  void AddWindow()
  {
    mWindows.push_back(std::make_unique<T>());
  }
};
