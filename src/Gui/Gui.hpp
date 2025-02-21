#pragma once

void GLFWInitialize();
GLFWwindow* GLFWCreateWindow(i32 width, i32 height, bool hidden = false);
void GLFWInitializeGL(GLFWwindow* window);
void GLFWSetWindowCallback(GLFWwindow* window, GLFWkeyfun callback);
ImGuiIO& ImGuiInitialize(GLFWwindow* window, float scale, const std::string& iniPath = "", const std::string& fontPath = "");
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
void ImGuiSetDarkTheme();
void ImGuiSetClassicTheme();
void ImGuiSetLightTheme();
void ImGuiSetHazelTheme();
void ImPlotUpdatePlotTheme();
