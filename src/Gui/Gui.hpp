#pragma once

struct ProgressStatus
{
  f32 progress = 0;
  std::string process;
};

void GLFWInitialize();
GLFWwindow* GLFWCreateWindow(i32 width, i32 height, bool hidden = false);
void GLFWInitializeGL(GLFWwindow* window);
void GLFWSetWindowCallback(GLFWwindow* window, GLFWkeyfun callback);
ImGuiIO& ImGuiInitialize(GLFWwindow* window, float scale);
void ImGuiNewFrame();
void ImGuiRender(GLFWwindow* window, ImGuiIO& io);
void RenderPlatformWindows(ImGuiIO& io);
void HandleIO(GLFWwindow* window);
void SwapBuffers(GLFWwindow* window);
void ImGuiShutdown();
void GLFWShutdown(GLFWwindow* window);
void GLFWCloseWindow(GLFWwindow* window);
