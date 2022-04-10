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
  io.Fonts->AddFontFromFileTTF("../data/apps/CascadiaCode.ttf", scale * 19);
  io.IniFilename = "../data/apps/imgui.ini";

  ImGui::StyleColorsClassic();
  ImPlot::StyleColorsClassic();
  ImGuiStyle& style = ImGui::GetStyle();
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
