#include "IPCOptimizeWindow.hpp"
#include "IPCWindow.hpp"

void IPCOptimizeWindow::Initialize()
{
}

void IPCOptimizeWindow::Render()
{
  if (ImGui::BeginTabItem("IPC opt"))
  {
    ImGui::Separator();
    ImGui::ProgressBar(mProgressStatus.progress, ImVec2(0.f, 0.f));
    ImGui::SameLine();
    ImGui::Text("%s", mProgressStatus.process.c_str());

    ImGui::Separator();
    if (ImGui::Button("Measure"))
      LaunchAsync(
          []()
          {
            IPCMeasure::MeasureAccuracy(IPCWindow::GetIPC(), IPCWindow::GetIPCOptimized(),
                fmt::format("../debug/ipcopt/imreg_dataset_{}x{}_{}i_{}ns", IPCWindow::GetIPC().GetCols(), IPCWindow::GetIPC().GetRows(),
                    mParameters.iters, mParameters.noiseStddev),
                &mProgressStatus.progress);
          });

    ImGui::Separator();

    ImGui::InputText("##opt train dir", &mParameters.trainDirectory);
    ImGui::SameLine();
    if (ImGui::Button("Optimize"))
      LaunchAsync(
          []()
          {
            IPCOptimization::Optimize(IPCWindow::GetIPCOptimized(), mParameters.trainDirectory, mParameters.trainDirectory, mParameters.maxShift,
                mParameters.noiseStddev, mParameters.optiters, mParameters.testRatio, mParameters.popSize);
          });

    ImGui::Separator();

    ImGui::InputText("##generate dataset dir", &mParameters.generateDirectory);
    ImGui::SameLine();
    if (ImGui::Button("Generate pairs"))
      LaunchAsync(
          []()
          {
            IPCMeasure::GenerateRegistrationDataset(IPCWindow::GetIPC(), mParameters.trainDirectory, mParameters.generateDirectory, mParameters.iters,
                mParameters.maxShift, mParameters.noiseStddev, &mProgressStatus.progress);
          });

    ImGui::Separator();

    ImGui::InputText("debug image1", &mParameters.debugImage1Path);
    ImGui::InputText("debug image2", &mParameters.debugImage2Path);
    if (ImGui::Button("DebugShift"))
      LaunchAsync([]() { IPCDebug::DebugShift(IPCWindow::GetIPCOptimized(), mParameters.maxShift, mParameters.noiseStddev); });
    ImGui::SameLine();
    if (ImGui::Button("DebugShift2"))
      LaunchAsync(
          []() {
            IPCDebug::DebugShift2(IPCWindow::GetIPCOptimized(), mParameters.debugImage1Path, mParameters.debugImage2Path, mParameters.noiseStddev);
          });
    ImGui::SameLine();
    if (ImGui::Button("DebugAlign"))
      LaunchAsync([]() { IPCDebug::DebugAlign(IPCWindow::GetIPCOptimized(), mParameters.noiseStddev); });
    ImGui::SameLine();
    if (ImGui::Button("DebugGradual"))
      LaunchAsync([]() { IPCDebug::DebugGradualShift(IPCWindow::GetIPCOptimized(), mParameters.maxShift, mParameters.noiseStddev); });

    ImGui::Separator();

    ImGui::SliderFloat("max shift", &mParameters.maxShift, 0.5, 3.0);
    ImGui::SliderFloat("noise stddev", &mParameters.noiseStddev, 0.0, 0.1, nullptr, ImGuiSliderFlags_Logarithmic);
    ImGui::SliderInt("iters", &mParameters.iters, 3, 201);
    ImGui::SliderInt("opt iters", &mParameters.optiters, 3, 201);
    ImGui::SliderFloat("test ratio", &mParameters.testRatio, 0.0, 1.0);
    ImGui::SliderInt("popsize", &mParameters.popSize, 6, 60);
    ImGui::EndTabItem();
  }
}
