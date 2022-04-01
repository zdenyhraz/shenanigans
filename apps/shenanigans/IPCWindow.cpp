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

  ImGui::SliderInt("Width", &mIPCPars.Cols, 3, 512);
  ImGui::SliderInt("Height", &mIPCPars.Rows, 3, 512);
  ImGui::SliderFloat("BPL", &mIPCPars.BPL, 0, 1);
  ImGui::SliderFloat("BPH", &mIPCPars.BPH, 0, 2);
  ImGui::SliderInt("L2size", &mIPCPars.L2size, 3, 11);
  ImGui::SliderFloat("L1ratio", &mIPCPars.L1ratio, 0.1, 0.9);
  ImGui::SliderInt("L2Usize", &mIPCPars.L2Usize, 31, 501);
  ImGui::SliderInt("MaxIter", &mIPCPars.MaxIter, 1, 21);
  ImGui::SliderFloat("CPeps", &mIPCPars.CPeps, 0, 0.1);
  ImGui::Combo("WindowType", &mIPCPars.WinT, IPCParameters::WindowTypes, IM_ARRAYSIZE(IPCParameters::WindowTypes));
  ImGui::Combo("BandpassType", &mIPCPars.BPT, IPCParameters::BandpassTypes, IM_ARRAYSIZE(IPCParameters::BandpassTypes));
  ImGui::Combo("InterpolationType", &mIPCPars.IntT, IPCParameters::InterpolationTypes, IM_ARRAYSIZE(IPCParameters::InterpolationTypes));
  ImGui::Combo("L1WindowType", &mIPCPars.L1WinT, IPCParameters::L1windowTypes, IM_ARRAYSIZE(IPCParameters::L1windowTypes));
  ImGui::End();
}

void IPCWindow::UpdateIPCParameters()
{
  mIPC.SetSize(mIPCPars.Rows, mIPCPars.Cols);
  mIPC.SetBandpassParameters(mIPCPars.BPL, mIPCPars.BPH);
  mIPC.SetL2size(mIPCPars.L2size);
  mIPC.SetL1ratio(mIPCPars.L1ratio);
  mIPC.SetL2Usize(mIPCPars.L2Usize);
  mIPC.SetMaxIterations(mIPCPars.MaxIter);
  mIPC.SetCrossPowerEpsilon(mIPCPars.CPeps);
  mIPC.SetWindowType(static_cast<IPC::WindowType>(mIPCPars.WinT));
  mIPC.SetBandpassType(static_cast<IPC::BandpassType>(mIPCPars.BPT));
  mIPC.SetInterpolationType(static_cast<IPC::InterpolationType>(mIPCPars.IntT));
  mIPC.SetL1WindowType(static_cast<IPC::L1WindowType>(mIPCPars.L1WinT));
  LOG_DEBUG("IPC parameters updated");
}
