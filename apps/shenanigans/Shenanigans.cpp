#include "Shenanigans.hpp"
#include "Gui/Gui.hpp"
#include "Python/Python.hpp"

void Shenanigans::Run()
{
  PythonInitialize();
  GLFWInitialize();
  auto window = GLFWCreateWindow();
  GLFWInitializeGL(window);
  GLFWSetWindowCallback(window, KeyCallback);
  ImGuiIO& io = ImGuiInitialize(window);

  UpdateIPCParameters();

  while (!glfwWindowShouldClose(window))
  {
    ImGuiNewFrame();

    {
      ImGui::Begin("IPC parameters");

      if (ImGui::Button("Update"))
        UpdateIPCParameters();
      ImGui::SameLine();

      if (ImGui::Button("Debug"))
        IPCDebug::ShowDebugStuff(mIPC);

      ImGui::SliderInt("Width", &mIPCParameters.mCols, 3, 4096);
      ImGui::SliderInt("Height", &mIPCParameters.mRows, 3, 4096);
      ImGui::SliderFloat("BPL", &mIPCParameters.mBPL, 0, 1);
      ImGui::SliderFloat("BPH", &mIPCParameters.mBPH, 0, 2);
      ImGui::SliderInt("L2size", &mIPCParameters.mL2size, 3, 11);
      ImGui::SliderFloat("L1ratio", &mIPCParameters.mL1ratio, 0.1, 0.9);
      ImGui::SliderInt("L2Usize", &mIPCParameters.mL2Usize, 31, 501);
      ImGui::SliderInt("MaxIter", &mIPCParameters.mMaxIter, 1, 21);
      ImGui::SliderFloat("CPeps", &mIPCParameters.mCPeps, 0, 0.1);
      ImGui::End();
    }

    ImGuiRender(window, io);
  }

  ImGuiShutdown();
  GLFWShutdown(window);
}

void Shenanigans::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void Shenanigans::UpdateIPCParameters()
{
  mIPC.SetSize(mIPCParameters.mRows, mIPCParameters.mCols);
  mIPC.SetBandpassParameters(mIPCParameters.mBPL, mIPCParameters.mBPH);
  mIPC.SetL2size(mIPCParameters.mL2size);
  mIPC.SetL1ratio(mIPCParameters.mL1ratio);
  mIPC.SetL2Usize(mIPCParameters.mL2Usize);
  mIPC.SetMaxIterations(mIPCParameters.mMaxIter);
  mIPC.SetCrossPowerEpsilon(mIPCParameters.mCPeps);
  LOG_DEBUG("IPC parameters updated");
}
