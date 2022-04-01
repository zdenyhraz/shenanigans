#pragma once

void GLFWInitialize();
GLFWwindow* GLFWCreateWindow(i32 width, i32 height);
void GLFWInitializeGL(GLFWwindow* window);
void GLFWSetWindowCallback(GLFWwindow* window, GLFWkeyfun callback);
ImGuiIO& ImGuiInitialize(GLFWwindow* window, float scale = 2);
void ImGuiNewFrame();
void ImGuiRender(GLFWwindow* window, ImGuiIO& io);
void ImGuiShutdown();
void GLFWShutdown(GLFWwindow* window);
void GLFWCloseWindow(GLFWwindow* window);
