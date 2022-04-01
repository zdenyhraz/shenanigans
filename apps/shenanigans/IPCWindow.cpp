#include "IPCWindow.hpp"

void IPCWindow::Initialize()
{
  UpdateIPCParameters();
}

void IPCWindow::Render()
{
  ImGui::Begin("IPC parameters");

  if (ImGui::Button("Update"))
    UpdateIPCParameters();

  ImGui::SameLine();

  if (ImGui::Button("Debug"))
    IPCDebug::ShowDebugStuff(mIPC);

  ImGui::SliderInt("Width", &mParameters.Cols, 3, 512);
  ImGui::SliderInt("Height", &mParameters.Rows, 3, 512);
  ImGui::SliderFloat("BPL", &mParameters.BPL, 0, 1);
  ImGui::SliderFloat("BPH", &mParameters.BPH, 0, 2);
  ImGui::SliderInt("L2size", &mParameters.L2size, 3, 11);
  ImGui::SliderFloat("L1ratio", &mParameters.L1ratio, 0.1, 0.9);
  ImGui::SliderInt("L2Usize", &mParameters.L2Usize, 31, 501);
  ImGui::SliderInt("MaxIter", &mParameters.MaxIter, 1, 21);
  ImGui::SliderFloat("CPeps", &mParameters.CPeps, 0, 0.1);
  ImGui::Combo("WindowType", &mParameters.WinT, IPCParameters::WindowTypes, IM_ARRAYSIZE(IPCParameters::WindowTypes));
  ImGui::Combo("BandpassType", &mParameters.BPT, IPCParameters::BandpassTypes, IM_ARRAYSIZE(IPCParameters::BandpassTypes));
  ImGui::Combo("InterpolationType", &mParameters.IntT, IPCParameters::InterpolationTypes, IM_ARRAYSIZE(IPCParameters::InterpolationTypes));
  ImGui::Combo("L1WindowType", &mParameters.L1WinT, IPCParameters::L1windowTypes, IM_ARRAYSIZE(IPCParameters::L1windowTypes));
  ImGui::End();
}

IPC& IPCWindow::GetIPC()
{
  return mIPC;
}

void IPCWindow::UpdateIPCParameters()
{
  mIPC.SetSize(mParameters.Rows, mParameters.Cols);
  mIPC.SetBandpassParameters(mParameters.BPL, mParameters.BPH);
  mIPC.SetL2size(mParameters.L2size);
  mIPC.SetL1ratio(mParameters.L1ratio);
  mIPC.SetL2Usize(mParameters.L2Usize);
  mIPC.SetMaxIterations(mParameters.MaxIter);
  mIPC.SetCrossPowerEpsilon(mParameters.CPeps);
  mIPC.SetWindowType(static_cast<IPC::WindowType>(mParameters.WinT));
  mIPC.SetBandpassType(static_cast<IPC::BandpassType>(mParameters.BPT));
  mIPC.SetInterpolationType(static_cast<IPC::InterpolationType>(mParameters.IntT));
  mIPC.SetL1WindowType(static_cast<IPC::L1WindowType>(mParameters.L1WinT));
  LOG_DEBUG("IPC parameters updated");
}
