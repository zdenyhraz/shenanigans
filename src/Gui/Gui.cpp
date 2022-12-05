#include "Gui.hpp"

void GLFWInitialize()
{
  PROFILE_FUNCTION;
  LOG_DEBUG("Initializing GLFW ...");
  auto GLFWErrorCallback = [](int error, const char* description) { LOG_ERROR("GLFWError {}: {}", error, description); };
  glfwSetErrorCallback(GLFWErrorCallback);
  if (not glfwInit())
    throw std::runtime_error("GLFW initialization failed");
}

GLFWwindow* GLFWCreateWindow(i32 width, i32 height, bool hidden)
{
  PROFILE_FUNCTION;
  LOG_DEBUG("Creating window ...");
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
  LOG_DEBUG("Initializing OpenGL ...");
  glfwMakeContextCurrent(window);
  gladLoadGL();
  glfwSwapInterval(1);
  LOG_DEBUG("OpenGL version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
}

void GLFWSetWindowCallback(GLFWwindow* window, GLFWkeyfun callback)
{
  glfwSetKeyCallback(window, callback);
}

void ImGuiSetStyle(ImGuiStyle& style)
{
  constexpr auto ColorFromBytes = [](f32 r, f32 g, f32 b) { return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f); };

  ImVec4* colors = style.Colors;

  const ImVec4 bgColor = ColorFromBytes(37, 37, 38);
  const ImVec4 lightBgColor = ColorFromBytes(82, 82, 85);
  const ImVec4 veryLightBgColor = ColorFromBytes(90, 90, 95);

  const ImVec4 panelColor = ColorFromBytes(51, 51, 55);
  const ImVec4 panelHoverColor = ColorFromBytes(29, 151, 236);
  const ImVec4 panelActiveColor = ColorFromBytes(0, 119, 200);

  const ImVec4 textColor = ColorFromBytes(255, 255, 255);
  const ImVec4 textDisabledColor = ColorFromBytes(151, 151, 151);
  const ImVec4 borderColor = ColorFromBytes(78, 78, 78);

  colors[ImGuiCol_Text] = textColor;
  colors[ImGuiCol_TextDisabled] = textDisabledColor;
  colors[ImGuiCol_TextSelectedBg] = panelActiveColor;
  colors[ImGuiCol_WindowBg] = bgColor;
  colors[ImGuiCol_ChildBg] = bgColor;
  colors[ImGuiCol_PopupBg] = bgColor;
  colors[ImGuiCol_Border] = borderColor;
  colors[ImGuiCol_BorderShadow] = borderColor;
  colors[ImGuiCol_FrameBg] = panelColor;
  colors[ImGuiCol_FrameBgHovered] = panelHoverColor;
  colors[ImGuiCol_FrameBgActive] = panelActiveColor;
  colors[ImGuiCol_TitleBg] = bgColor;
  colors[ImGuiCol_TitleBgActive] = bgColor;
  colors[ImGuiCol_TitleBgCollapsed] = bgColor;
  colors[ImGuiCol_MenuBarBg] = panelColor;
  colors[ImGuiCol_ScrollbarBg] = panelColor;
  colors[ImGuiCol_ScrollbarGrab] = lightBgColor;
  colors[ImGuiCol_ScrollbarGrabHovered] = veryLightBgColor;
  colors[ImGuiCol_ScrollbarGrabActive] = veryLightBgColor;
  colors[ImGuiCol_CheckMark] = panelActiveColor;
  colors[ImGuiCol_SliderGrab] = panelHoverColor;
  colors[ImGuiCol_SliderGrabActive] = panelActiveColor;
  colors[ImGuiCol_Button] = panelColor;
  colors[ImGuiCol_ButtonHovered] = panelHoverColor;
  colors[ImGuiCol_ButtonActive] = panelHoverColor;
  colors[ImGuiCol_Header] = panelColor;
  colors[ImGuiCol_HeaderHovered] = panelHoverColor;
  colors[ImGuiCol_HeaderActive] = panelActiveColor;
  colors[ImGuiCol_Separator] = borderColor;
  colors[ImGuiCol_SeparatorHovered] = borderColor;
  colors[ImGuiCol_SeparatorActive] = borderColor;
  colors[ImGuiCol_ResizeGrip] = bgColor;
  colors[ImGuiCol_ResizeGripHovered] = panelColor;
  colors[ImGuiCol_ResizeGripActive] = lightBgColor;
  colors[ImGuiCol_PlotLines] = panelActiveColor;
  colors[ImGuiCol_PlotLinesHovered] = panelHoverColor;
  colors[ImGuiCol_PlotHistogram] = panelActiveColor;
  colors[ImGuiCol_PlotHistogramHovered] = panelHoverColor;
  colors[ImGuiCol_ModalWindowDimBg] = bgColor;
  colors[ImGuiCol_DragDropTarget] = bgColor;
  colors[ImGuiCol_NavHighlight] = bgColor;
  colors[ImGuiCol_DockingPreview] = panelActiveColor;
  colors[ImGuiCol_Tab] = bgColor;
  colors[ImGuiCol_TabActive] = panelActiveColor;
  colors[ImGuiCol_TabUnfocused] = bgColor;
  colors[ImGuiCol_TabUnfocusedActive] = panelActiveColor;
  colors[ImGuiCol_TabHovered] = panelHoverColor;

  style.WindowRounding = 0.0f;
  style.ChildRounding = 0.0f;
  style.FrameRounding = 0.0f;
  style.GrabRounding = 0.0f;
  style.PopupRounding = 0.0f;
  style.ScrollbarRounding = 0.0f;
  style.TabRounding = 0.0f;
}

ImGuiIO& ImGuiInitialize(GLFWwindow* window, float scale)
{
  PROFILE_FUNCTION;
  LOG_DEBUG("Initializing ImGui ...");
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
  io.Fonts->AddFontFromFileTTF((GetProjectDirectoryPath() / "data/apps/CascadiaCode.ttf").string().c_str(), scale * 12);
  static const std::string iniFilename = (GetProjectDirectoryPath() / "data/apps/imgui.ini").string();
  io.IniFilename = iniFilename.c_str();

  ImGui::StyleColorsDark();
  ImPlot::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  ImGuiSetStyle(style);
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
  glfwPollEvents();
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImGuiRender(GLFWwindow* window, ImGuiIO& io)
{
  ImGui::Render();
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);
  glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    GLFWwindow* backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }

  if (ImGui::IsKeyPressed(ImGuiKey_Escape))
    GLFWCloseWindow(window);

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
