#include "Shenanigans.hpp"
#include "Gui/Gui.hpp"
#include "Python/Python.hpp"

void Shenanigans::Run()
{
  PythonInitialize();
  GLFWInitialize();
  auto window = GLFWCreateWindow(1.5 * 1920, 1.5 * 1080);
  GLFWInitializeGL(window);
  GLFWSetWindowCallback(window, KeyCallback);
  ImGuiIO& io = ImGuiInitialize(window, 2.0);

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

      ImGui::SliderInt("Width", &mIPCPars.mCols, 3, 512);
      ImGui::SliderInt("Height", &mIPCPars.mRows, 3, 512);
      ImGui::SliderFloat("BPL", &mIPCPars.mBPL, 0, 1);
      ImGui::SliderFloat("BPH", &mIPCPars.mBPH, 0, 2);
      ImGui::SliderInt("L2size", &mIPCPars.mL2size, 3, 11);
      ImGui::SliderFloat("L1ratio", &mIPCPars.mL1ratio, 0.1, 0.9);
      ImGui::SliderInt("L2Usize", &mIPCPars.mL2Usize, 31, 501);
      ImGui::SliderInt("MaxIter", &mIPCPars.mMaxIter, 1, 21);
      ImGui::SliderFloat("CPeps", &mIPCPars.mCPeps, 0, 0.1);
      ImGui::Combo("WindowType", &mIPCPars.mWinT, IPCParameters::mWindowTypes, IM_ARRAYSIZE(IPCParameters::mWindowTypes));
      ImGui::Combo("BandpassType", &mIPCPars.mBPT, IPCParameters::mBandpassTypes, IM_ARRAYSIZE(IPCParameters::mBandpassTypes));
      ImGui::Combo("InterpolationType", &mIPCPars.mIntT, IPCParameters::mInterpolationTypes, IM_ARRAYSIZE(IPCParameters::mInterpolationTypes));
      ImGui::Combo("L1WindowType", &mIPCPars.mL1WinT, IPCParameters::mL1windowTypes, IM_ARRAYSIZE(IPCParameters::mL1windowTypes));
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
  mIPC.SetSize(mIPCPars.mRows, mIPCPars.mCols);
  mIPC.SetBandpassParameters(mIPCPars.mBPL, mIPCPars.mBPH);
  mIPC.SetL2size(mIPCPars.mL2size);
  mIPC.SetL1ratio(mIPCPars.mL1ratio);
  mIPC.SetL2Usize(mIPCPars.mL2Usize);
  mIPC.SetMaxIterations(mIPCPars.mMaxIter);
  mIPC.SetCrossPowerEpsilon(mIPCPars.mCPeps);
  mIPC.SetWindowType(static_cast<IPC::WindowType>(mIPCPars.mWinT));
  mIPC.SetBandpassType(static_cast<IPC::BandpassType>(mIPCPars.mBPT));
  mIPC.SetInterpolationType(static_cast<IPC::InterpolationType>(mIPCPars.mIntT));
  mIPC.SetL1WindowType(static_cast<IPC::L1WindowType>(mIPCPars.mL1WinT));
  LOG_DEBUG("IPC parameters updated");
}
