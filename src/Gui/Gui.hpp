#pragma once

void GLFWInitialize();
GLFWwindow* GLFWCreateWindow(int width, int height, bool hidden = false);
void GLFWInitializeGL(GLFWwindow* window);
void GLFWSetWindowCallback(GLFWwindow* window, GLFWkeyfun callback);
ImGuiIO& ImGuiInitialize(GLFWwindow* window, float scale, const std::filesystem::path& iniPath = "", const std::filesystem::path& fontPath = "");
void ImGuiSetDefaultStyle();
void ImGuiNewFrame();
void ImGuiRender(GLFWwindow* window, ImGuiIO& io);
void RenderPlatformWindows(ImGuiIO& io);
void HandleIO(GLFWwindow* window);
void SwapBuffers(GLFWwindow* window);
void ImGuiShutdown();
void GLFWShutdown(GLFWwindow* window);
void GLFWCloseWindow(GLFWwindow* window);
void GLFWSetWindowIcon(GLFWwindow* window, int width, int height, uchar* data);
void ImGuiSetDarkTheme();
void ImGuiSetClassicTheme();
void ImGuiSetLightTheme();
void ImGuiSetHazelTheme();
void ImPlotUpdatePlotTheme();
void LogGPUInfo();
void LogMonitorInfo();
void LogImGuiInfo();
float GetResolutionScale();
