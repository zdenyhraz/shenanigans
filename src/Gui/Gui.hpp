#pragma once

void GLFWInitialize();
GLFWwindow* GLFWCreateWindow();
void GLFWInitializeGL(GLFWwindow* window);
void GLFWSetWindowCallback(GLFWwindow* window, GLFWkeyfun callback);
ImGuiIO& ImGuiInitialize(GLFWwindow* window);
void ImGuiNewFrame();
void ImGuiRender(GLFWwindow* window, ImGuiIO& io);
void ImGuiShutdown();
void GLFWShutdown(GLFWwindow* window);
