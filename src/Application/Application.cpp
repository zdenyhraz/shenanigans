#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include "Windows/Shenanigans/WindowShenanigans.hpp"

void QtInitialize()
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
}

void PythonInitialize()
{
  static py::scoped_interpreter guard{};
}

static void GLFWErrorCallback(int error, const char* description)
{
  LOG_ERROR("GLFW Error: {}", description);
}

// static void GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
// {
//   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//     glfwSetWindowShouldClose(window, GLFW_TRUE);
// }

int main(int argc, char** argv)
try
{
  // fmt::print("{}", glGetString(GL_VERSION));

  {
    glfwSetErrorCallback(GLFWErrorCallback);
    if (not glfwInit())
      throw std::runtime_error("GLFW initialization failed");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "xdd", NULL, NULL);
    if (not window)
      throw std::runtime_error("GLFW create window failed");
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, GLFWKeyCallback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;
    ImGui::StyleColorsDark(); // ImGui::StyleColorsClassic();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
      style.WindowRounding = 0.0f;
      style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    while (!glfwWindowShouldClose(window))
    {
      glfwPollEvents();
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      {
        ImGui::Begin("ddx");
        ImGui::Button("Hi mom");
        static float value = 0;
        ImGui::DragFloat("value", &value);
        ImGui::End();
      }

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

      glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
  }

  QtInitialize();
  PythonInitialize();
  QApplication application(argc, argv);
  WindowShenanigans windowshen;
  windowshen.show();
  application.exec();

  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  fmt::print("Error: {}\n", e.what());
  return EXIT_FAILURE;
}
catch (...)
{
  fmt::print("Error: Unknown error\n");
  return EXIT_FAILURE;
}
