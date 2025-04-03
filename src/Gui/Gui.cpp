#include "Gui.hpp"

void GLFWInitialize()
{
  PROFILE_FUNCTION;
  static auto GLFWErrorCallback = [](int error, const char* description) { LOG_ERROR("GLFWError {}: {}", error, description); };
  glfwSetErrorCallback(GLFWErrorCallback);
  if (not glfwInit())
    throw std::runtime_error("GLFW initialization failed");
  LOG_DEBUG("GLFW version: {}", glfwGetVersionString());
}

GLFWwindow* GLFWCreateWindow(int width, int height, bool hidden)
{
  PROFILE_FUNCTION;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_VISIBLE, hidden ? GLFW_FALSE : GLFW_TRUE);
  GLFWwindow* window = glfwCreateWindow(hidden ? 1 : width, hidden ? 1 : height, "hi mom", nullptr, nullptr);
  if (not window)
    throw std::runtime_error("GLFW create window failed");

  return window;
}

void GLFWInitializeGL(GLFWwindow* window)
{
  PROFILE_FUNCTION;
  glfwMakeContextCurrent(window);
  gladLoadGL();
  glfwSwapInterval(1);
  LOG_DEBUG("OpenGL version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
}

void GLFWSetWindowCallback(GLFWwindow* window, GLFWkeyfun callback)
{
  glfwSetKeyCallback(window, callback);
}

void ImGuiSetDefaultStyle()
{
  PROFILE_FUNCTION;
  ImGuiSetHazelTheme();
}

void LogGPUInfo()
{
  int vramTotal, vramAvailable;
  glGetIntegerv(0x9048, &vramTotal);
  glGetIntegerv(0x9049, &vramAvailable);
  LOG_DEBUG("GPU: {}, VRAM available: {:.1f}GB / {:.1f}GB ({:.0f}%)", reinterpret_cast<const char*>(glGetString(GL_RENDERER)), static_cast<float>(vramAvailable) / 1024 / 1024,
      static_cast<float>(vramTotal) / 1024 / 1024, static_cast<float>(vramAvailable) / vramTotal * 100);
}

void LogMonitorInfo()
{
  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  const char* monitorName = glfwGetMonitorName(monitor);
  const float resolutionScale = static_cast<float>(mode->width) / 1920;
  LOG_DEBUG("Monitor: {}, {}x{}, {}Hz, resolution scale: {}", monitorName, mode->width, mode->height, mode->refreshRate, resolutionScale);
}

void LogImGuiInfo()
{
  LOG_DEBUG("ImGui version: {}", IMGUI_VERSION);
  LOG_DEBUG("ImPlot version: {}", IMPLOT_VERSION);
}

float GetResolutionScale()
{
  return static_cast<float>(glfwGetVideoMode(glfwGetPrimaryMonitor())->width) / 1920;
}

ImGuiIO& ImGuiInitialize(GLFWwindow* window, float baseScale, const std::filesystem::path& iniPath, const std::filesystem::path& fontPath)
{
  PROFILE_FUNCTION;
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  LogImGuiInfo();
  LogGPUInfo();
  LogMonitorInfo();

  const float scale = baseScale * GetResolutionScale();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows

  if (std::filesystem::exists(fontPath))
  {
    LOG_DEBUG("Using font '{}'", fontPath);
    static constexpr float fontSize = 10;
    io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), fontSize * scale);
  }
  else
  {
    LOG_WARNING("Font file '{}' not found", fontPath);
    io.FontGlobalScale = scale;
  }

  if (std::filesystem::exists(iniPath))
  {
    LOG_DEBUG("Using ini file '{}'", iniPath);
    static std::string iniPathStr = iniPath.string();
    io.IniFilename = iniPathStr.c_str();
  }
  else
  {
    LOG_WARNING("Ini file '{}' not found", iniPath);
  }

  ImGuiStyle& style = ImGui::GetStyle();
  ImGuiSetDefaultStyle();
  style.ScaleAllSizes(scale);
  style.GrabRounding = 12;

  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  ImPlotStyle& plotStyle = ImPlot::GetStyle();
  plotStyle.LineWeight = 7;
  plotStyle.Colormap = ImPlotColormap_Dark;
  // plotStyle.PlotDefaultSize = ImVec2(800, 800);

  if (not ImGui_ImplGlfw_InitForOpenGL(window, true))
    throw std::runtime_error("Failed to initialize ImGui GLFW");
  if (not ImGui_ImplOpenGL3_Init("#version 130"))
    throw std::runtime_error("Failed to initialize ImGui OpenGL");

  return io;
}

void ImGuiNewFrame()
{
  PROFILE_FUNCTION;
  glfwPollEvents();
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImGuiRender(GLFWwindow* window, ImGuiIO& io)
{
  PROFILE_FUNCTION;
  ImGui::Render();
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);
  glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  RenderPlatformWindows(io);
  HandleIO(window);
  SwapBuffers(window);
}

void RenderPlatformWindows(ImGuiIO& io)
{
  PROFILE_FUNCTION;
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    GLFWwindow* backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }
}

void HandleIO(GLFWwindow* window)
{
  if (ImGui::IsKeyPressed(ImGuiKey_Escape))
    GLFWCloseWindow(window);
}

void SwapBuffers(GLFWwindow* window)
{
  PROFILE_FUNCTION;
  glfwSwapBuffers(window);
}

void ImGuiShutdown()
{
  PROFILE_FUNCTION;
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
}

void GLFWShutdown(GLFWwindow* window)
{
  PROFILE_FUNCTION;
  glfwDestroyWindow(window);
  glfwTerminate();
}

void GLFWCloseWindow(GLFWwindow* window)
{
  glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void GLFWSetWindowIcon(GLFWwindow* window, int width, int height, uchar* data)
{
  GLFWimage icon;
  icon.width = width;
  icon.height = height;
  icon.pixels = data;
  glfwSetWindowIcon(window, 1, &icon);
}

void ImGuiSetDarkTheme()
{
  ImGui::StyleColorsDark();
  ImPlotUpdatePlotTheme();
}

void ImGuiSetClassicTheme()
{
  ImGui::StyleColorsClassic();
  ImPlotUpdatePlotTheme();
}

void ImGuiSetLightTheme()
{
  ImGui::StyleColorsLight();
  ImPlotUpdatePlotTheme();
}

void ImGuiSetHazelTheme()
{
  ImGui::StyleColorsDark();

  auto& colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

  // Headers
  colors[ImGuiCol_Header] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
  colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
  colors[ImGuiCol_HeaderActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  // Buttons
  colors[ImGuiCol_Button] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
  colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
  colors[ImGuiCol_ButtonActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  // Frame BG
  colors[ImGuiCol_FrameBg] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
  colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
  colors[ImGuiCol_FrameBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  // Tabs
  colors[ImGuiCol_Tab] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TabHovered] = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
  colors[ImGuiCol_TabActive] = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
  colors[ImGuiCol_TabUnfocused] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

  // Title
  colors[ImGuiCol_TitleBg] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TitleBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  ImPlotUpdatePlotTheme();
}

void ImPlotUpdatePlotTheme()
{
  ImPlot::StyleColorsAuto();
  auto& plotStyle = ImPlot::GetStyle();
  plotStyle.MinorAlpha = 0.25f;
  plotStyle.Colors[ImPlotCol_FrameBg] = ImVec4(0, 0, 0, 0);
  plotStyle.Colors[ImPlotCol_AxisBg] = ImVec4(0, 0, 0, 0);
  plotStyle.Colors[ImPlotCol_AxisBgHovered] = ImVec4(0, 0, 0, 0);
}
