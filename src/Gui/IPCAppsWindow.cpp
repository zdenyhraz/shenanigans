#include "IPCAppsWindow.hpp"
#include "IPCWindow.hpp"
#include "ImageRegistration/ImageRegistrationDataset.hpp"

void IPCAppsWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("IPC apps"))
  {
    ImGui::Separator();
    ImGui::SameLine();
    ImGui::Text("Accuracy measurement");
    if (ImGui::Button("Measure"))
      LaunchAsync([&]() { IPCMeasure::MeasureAccuracy(mIPC, mIPCOptimized, GetCurrentDatasetPath()); });

    ImGui::Separator();
    ImGui::Text("Debug uils");
    if (ImGui::Button("DebugShift"))
      LaunchAsync([&]() { IPCDebug::DebugShift(mIPCOptimized, mParameters.maxShift, mParameters.noiseStddev); });
    ImGui::SameLine();
    if (ImGui::Button("DebugShift2"))
      LaunchAsync([&]() { IPCDebug::DebugShift2(mIPCOptimized, mParameters.debugImage1Path, mParameters.debugImage2Path, mParameters.noiseStddev); });
    ImGui::SameLine();
    if (ImGui::Button("DebugAlign"))
      LaunchAsync([&]() { IPCDebug::DebugAlign(mIPCOptimized, mParameters.debugImage1Path, mParameters.debugImage2Path, mParameters.noiseStddev); });
    ImGui::SameLine();
    if (ImGui::Button("DebugGradual"))
      LaunchAsync([&]() { IPCDebug::DebugGradualShift(mIPCOptimized, mParameters.maxShift, mParameters.noiseStddev); });

    ImGui::Separator();
    ImGui::Text("IPC optimization");
    ImGui::SameLine();
    if (ImGui::Button("Optimize"))
      LaunchAsync([&]() { IPCOptimization::Optimize(mIPCOptimized, GetCurrentDatasetPath(), mParameters.popSize); });
    ImGui::InputText("image dir", &mParameters.imageDirectory);
    ImGui::InputText("##generate dataset dir", &mParameters.generateDirectory);
    ImGui::SameLine();
    if (ImGui::Button("Generate pairs"))
      LaunchAsync([&]()
          { GenerateImageRegistrationDataset(mIPC, mParameters.imageDirectory, mParameters.generateDirectory, mParameters.iters, mParameters.maxShift, mParameters.noiseStddev); });
    ImGui::InputText("debug image1", &mParameters.debugImage1Path);
    ImGui::InputText("debug image2", &mParameters.debugImage2Path);
    ImGui::SliderFloat("max shift", &mParameters.maxShift, 0.5, 3.0);
    ImGui::SliderFloat("noise stddev", &mParameters.noiseStddev, 0.0, 0.1, nullptr, ImGuiSliderFlags_Logarithmic);
    ImGui::SliderInt("iters", &mParameters.iters, 3, 201);
    ImGui::SliderFloat("test ratio", &mParameters.testRatio, 0.0, 1.0);
    ImGui::SliderInt("popsize", &mParameters.popSize, 6, 60);

    ImGui::Separator();
    ImGui::Text("False correlations removal");
    if (ImGui::Button("IPC/FCR"))
      LaunchAsync([&]() { FalseCorrelationsRemoval(); });
    ImGui::EndTabItem();
  }
}

std::string IPCAppsWindow::GetCurrentDatasetPath()
{
  return fmt::format("../debug/ipcopt/imreg_dataset_{}x{}_{}i_{:.3f}ns", mIPC.GetCols(), mIPC.GetRows(), mParameters.iters, mParameters.noiseStddev);
}

void IPCAppsWindow::FalseCorrelationsRemoval()
{
  LOG_FUNCTION;
  auto image1 = LoadUnitFloatImage<IPC::Float>("../data/articles/swind/source/1/cropped/crop1.PNG");
  auto image2 = LoadUnitFloatImage<IPC::Float>("../data/articles/swind/source/1/cropped/crop9.PNG");

  if (false) // crop
  {
    const auto centerx = 0.1;
    const auto centery = 0.7;
    const auto size = 2 * centerx * image1.cols;
    image1 = RoiCrop(image1, centerx * image1.cols, centery * image1.rows, size, size);
    image2 = RoiCrop(image2, centerx * image2.cols, centery * image2.rows, size, size);
  }
  mIPC.SetSize(image1.size());
  mIPC.Calculate<IPC::Mode::Debug>(image1, image2);
}
