#include "Gui.hpp"

void GLFWInitialize()
{
  PROFILE_FUNCTION;
  LOG_TRACE("Initializing GLFW");
  auto GLFWErrorCallback = [](int error, const char* description) { LOG_ERROR("GLFWError {}: {}", error, description); };
  glfwSetErrorCallback(GLFWErrorCallback);
  if (not glfwInit())
    throw std::runtime_error("GLFW initialization failed");
}

GLFWwindow* GLFWCreateWindow(i32 width, i32 height, bool hidden)
{
  PROFILE_FUNCTION;
  LOG_TRACE("Creating window");
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
  LOG_TRACE("Initializing OpenGL");
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
  LOG_TRACE("Setting ImGui style");
  ImGuiSetHazelTheme();
}

ImGuiIO& ImGuiInitialize(GLFWwindow* window, float scale)
{
  PROFILE_FUNCTION;
  LOG_TRACE("Initializing ImGui");
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
  io.Fonts->AddFontFromFileTTF((GetProjectDirectoryPath() / "data/apps/CascadiaCode.ttf").string().c_str(), scale * 10);
  static const std::string iniFilename = (GetProjectDirectoryPath() / "data/apps/imgui.ini").string();
  io.IniFilename = iniFilename.c_str();

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

void GLFWSetWindowIcon(GLFWwindow* window, i32 width, i32 height, uchar* data)
{
  LOG_TRACE("Setting window icon");
  GLFWimage image;
  image.width = width;
  image.height = height;
  image.pixels = data;
  glfwSetWindowIcon(window, 1, &image);
}

void CheckGLError(std::string_view name)
{
  const auto error = glGetError();
  switch (error)
  {
  case GL_NO_ERROR:
    return; // No error has been recorded.The value of this symbolic constant is guaranteed to be 0.

  case GL_INVALID_ENUM:
    return LOG_WARNING("{} GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than "
                       "to set the error flag.",
        name);

  case GL_INVALID_VALUE:
    return LOG_WARNING("{} GL_INVALID_VALUE: A numeric argument is out of range.The offending command is ignored and has no other side effect than to set the error flag.", name);

  case GL_INVALID_OPERATION:
    return LOG_WARNING("{} GL_INVALID_OPERATION: The specified operation is not allowed in the current state.The offending command is ignored and has no other side effect than "
                       "to set the error flag.",
        name);

  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return LOG_WARNING(
        "{} GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.The offending command is ignored and has no other side effect than to set the error flag.",
        name);

  case GL_OUT_OF_MEMORY:
    return LOG_WARNING("{} GL_OUT_OF_MEMORY: There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, "
                       "after this error is recorded.",
        name);

  case GL_STACK_UNDERFLOW:
    return LOG_WARNING("{} GL_STACK_UNDERFLOW: An attempt has been made to perform an operation that would cause an internal stack to underflow.", name);

  case GL_STACK_OVERFLOW:
    return LOG_WARNING("{} GL_STACK_OVERFLOW: An attempt has been made to perform an operation that would cause an internal stack to overflow.", name);
  }
  return LOG_WARNING("{} GL_UNKNOWN_ERROR: {}", name, error);
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
  auto& style = ImGui::GetStyle();
  auto& colors = style.Colors;
  auto& plotStyle = ImPlot::GetStyle();
  auto& plotColors = plotStyle.Colors;

  plotStyle.MinorAlpha = 0.25f;
  plotColors[ImPlotCol_Line] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_Fill] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_MarkerOutline] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_MarkerFill] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_ErrorBar] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_FrameBg] = ImVec4(0, 0, 0, 0);
  plotColors[ImPlotCol_PlotBg] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_PlotBorder] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_LegendBg] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_LegendBorder] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_LegendText] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_TitleText] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_InlayText] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_PlotBorder] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_AxisText] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_AxisGrid] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_AxisTick] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_AxisBg] = ImVec4(0, 0, 0, 0);
  plotColors[ImPlotCol_AxisBgHovered] = ImVec4(0, 0, 0, 0);
  plotColors[ImPlotCol_AxisBgActive] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_Selection] = IMPLOT_AUTO_COL;
  plotColors[ImPlotCol_Crosshairs] = IMPLOT_AUTO_COL;
}
