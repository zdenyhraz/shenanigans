#include "IPCWindow.hpp"

void IPCWindow::Initialize()
{
  UpdateIPCParameters(mIPC);
  UpdateIPCParameters(mIPCOptimized);
}

void IPCWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("IPC params"))
  {
    ImGui::Separator();

    if (ImGui::Button("Update"))
      LaunchAsync(
          [&]()
          {
            UpdateIPCParameters(mIPC);
            UpdateIPCParameters(mIPCOptimized);
          });

    ImGui::SliderInt("Width", &mParameters.Cols, 3, 4096, nullptr, ImGuiSliderFlags_Logarithmic);
    ImGui::SliderInt("Height", &mParameters.Rows, 3, 4096, nullptr, ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("BPL", &mParameters.BPL, 0, 1);
    ImGui::SliderFloat("BPH", &mParameters.BPH, 0, 2);
    ImGui::SliderInt("L2size", &mParameters.L2size, 3, 11);
    ImGui::SliderFloat("L1ratio", &mParameters.L1ratio, 0.1, 0.9);
    ImGui::SliderInt("L2Usize", &mParameters.L2Usize, 31, 501);
    ImGui::SliderInt("MaxIter", &mParameters.MaxIter, 1, 21);
    ImGui::SliderFloat("CPeps", &mParameters.CPeps, 0, 10, nullptr, ImGuiSliderFlags_Logarithmic);
    ImGui::SliderInt("WindowType", &mParameters.WinT, 0, static_cast<i32>(IPC::WindowType::WindowTypeCount) - 1, IPCParameters::WindowTypes[mParameters.WinT]);
    ImGui::SliderInt("BandpassType", &mParameters.BPT, 0, static_cast<i32>(IPC::BandpassType::BandpassTypeCount) - 1, IPCParameters::BandpassTypes[mParameters.BPT]);
    ImGui::SliderInt(
        "InterpolationType", &mParameters.IntT, 0, static_cast<i32>(IPC::InterpolationType::InterpolationTypeCount) - 1, IPCParameters::InterpolationTypes[mParameters.IntT]);
    ImGui::SliderInt("L1WindowType", &mParameters.L1WinT, 0, static_cast<i32>(IPC::L1WindowType::L1WindowTypeCount) - 1, IPCParameters::L1WindowTypes[mParameters.L1WinT]);
    ImGui::EndTabItem();
  }
}

void IPCWindow::UpdateIPCParameters(IPC& ipc)
{
  ipc.SetSize(mParameters.Rows, mParameters.Cols);
  ipc.SetBandpassParameters(mParameters.BPL, mParameters.BPH);
  ipc.SetL2size(mParameters.L2size);
  ipc.SetL1ratio(mParameters.L1ratio);
  ipc.SetL2Usize(mParameters.L2Usize);
  ipc.SetMaxIterations(mParameters.MaxIter);
  ipc.SetCrossPowerEpsilon(mParameters.CPeps);
  ipc.SetWindowType(static_cast<IPC::WindowType>(mParameters.WinT));
  ipc.SetBandpassType(static_cast<IPC::BandpassType>(mParameters.BPT));
  ipc.SetInterpolationType(static_cast<IPC::InterpolationType>(mParameters.IntT));
  ipc.SetL1WindowType(static_cast<IPC::L1WindowType>(mParameters.L1WinT));
}
