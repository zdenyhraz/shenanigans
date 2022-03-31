#include "Gui/Gui.hpp"
#include "Python/Python.hpp"

static void GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(int argc, char** argv)
try
{
  LOG_DEBUG("hi mom");
  PythonInitialize();
  GLFWInitialize();
  auto window = GLFWCreateWindow();
  GLFWInitializeGL(window);
  GLFWSetWindowCallback(window, GLFWKeyCallback);
  ImGuiIO& io = ImGuiInitialize(window);

  while (!glfwWindowShouldClose(window))
  {
    ImGuiNewFrame();

    {
      ImGui::Begin("ddx");
      ImGui::Button("Hi mom");
      static float value = 0;
      ImGui::DragFloat("value", &value);
      ImGui::End();
    }

    {
      ImGui::Begin("xxd");
      ImGui::Button("Hi mom");
      static float value = 0;
      ImGui::DragFloat("value", &value);
      ImGui::End();
    }

    ImGuiRender(window, io);
  }

  ImGuiShutdown();
  GLFWShutdown(window);
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
