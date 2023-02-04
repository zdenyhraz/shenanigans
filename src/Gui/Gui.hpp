#pragma once

void GLFWInitialize();
GLFWwindow* GLFWCreateWindow(i32 width, i32 height, bool hidden = false);
void GLFWInitializeGL(GLFWwindow* window);
void GLFWSetWindowCallback(GLFWwindow* window, GLFWkeyfun callback);
ImGuiIO& ImGuiInitialize(GLFWwindow* window, float scale);
void ImGuiSetDefaultStyle();
void ImGuiNewFrame();
void ImGuiRender(GLFWwindow* window, ImGuiIO& io);
void RenderPlatformWindows(ImGuiIO& io);
void HandleIO(GLFWwindow* window);
void SwapBuffers(GLFWwindow* window);
void ImGuiShutdown();
void GLFWShutdown(GLFWwindow* window);
void GLFWCloseWindow(GLFWwindow* window);
void GLFWSetWindowIcon(GLFWwindow* window, i32 width, i32 height, uchar* data);
void CheckGLError(std::string_view name);
void ImGuiSetDarkTheme();
void ImGuiSetClassicTheme();
void ImGuiSetDeepDarkTheme();
void ImGuiSetHazelTheme();
void ImGuiSetLightTheme();
