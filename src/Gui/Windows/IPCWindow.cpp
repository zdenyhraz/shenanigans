#include "IPCWindow.hpp"
#include "ImageRegistration/ImageRegistrationDataset.hpp"

void IPCWindow::Initialize()
{
  UpdateIPCParameters(mIPC);
  UpdateIPCParameters(mIPCOptimized);
}

void IPCWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("IPC"))
  {
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Iterative Phase correlation parameters"))
    {
      if (ImGui::Button("Update"))
        LaunchAsync(
            [&]()
            {
              UpdateIPCParameters(mIPC);
              UpdateIPCParameters(mIPCOptimized);
            });

      ImGui::SliderInt("Width", &mIPCParameters.Cols, 3, 4096, nullptr, ImGuiSliderFlags_Logarithmic);
      ImGui::SliderInt("Height", &mIPCParameters.Rows, 3, 4096, nullptr, ImGuiSliderFlags_Logarithmic);
      ImGui::SliderFloat("BPL", &mIPCParameters.BPL, 0, 1);
      ImGui::SliderFloat("BPH", &mIPCParameters.BPH, 0, 2);
      ImGui::SliderInt("L2size", &mIPCParameters.L2size, 3, 11);
      ImGui::SliderFloat("L1ratio", &mIPCParameters.L1ratio, 0.1, 0.9);
      ImGui::SliderInt("L2Usize", &mIPCParameters.L2Usize, 31, 501);
      ImGui::SliderInt("MaxIter", &mIPCParameters.MaxIter, 1, 21);
      ImGui::SliderFloat("CPeps", &mIPCParameters.CPeps, 0, 10, nullptr, ImGuiSliderFlags_Logarithmic);
      ImGui::SliderInt("WindowType", &mIPCParameters.WinT, 0, static_cast<i32>(IPC::WindowType::WindowTypeCount) - 1, IPCParameters::WindowTypes[mIPCParameters.WinT]);
      ImGui::SliderInt("BandpassType", &mIPCParameters.BPT, 0, static_cast<i32>(IPC::BandpassType::BandpassTypeCount) - 1, IPCParameters::BandpassTypes[mIPCParameters.BPT]);
      ImGui::SliderInt("InterpolationType", &mIPCParameters.IntT, 0, static_cast<i32>(IPC::InterpolationType::InterpolationTypeCount) - 1,
          IPCParameters::InterpolationTypes[mIPCParameters.IntT]);
      ImGui::SliderInt("L1WindowType", &mIPCParameters.L1WinT, 0, static_cast<i32>(IPC::L1WindowType::L1WindowTypeCount) - 1, IPCParameters::L1WindowTypes[mIPCParameters.L1WinT]);
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Iterative Phase correlation utils"))
    {
      ImGui::BulletText("Debug utils");
      if (ImGui::Button("DebugShift"))
        LaunchAsync([&]() { IPCDebug::DebugShift(mIPCOptimized, mOptimizeParameters.maxShift, mOptimizeParameters.noiseStddev); });
      ImGui::SameLine();
      if (ImGui::Button("DebugShift2"))
        LaunchAsync(
            [&]()
            {
              IPCDebug::DebugShift2(mIPCOptimized, GetProjectDirectoryPath(mOptimizeParameters.debugImage1Path).string(),
                  GetProjectDirectoryPath(mOptimizeParameters.debugImage2Path).string(), mOptimizeParameters.noiseStddev);
            });
      ImGui::SameLine();
      if (ImGui::Button("DebugAlign"))
        LaunchAsync(
            [&]()
            {
              IPCDebug::DebugAlign(mIPCOptimized, GetProjectDirectoryPath(mOptimizeParameters.debugImage1Path).string(),
                  GetProjectDirectoryPath(mOptimizeParameters.debugImage2Path).string(), mOptimizeParameters.noiseStddev);
            });
      ImGui::SameLine();
      if (ImGui::Button("DebugGradual"))
        LaunchAsync([&]() { IPCDebug::DebugGradualShift(mIPCOptimized, mOptimizeParameters.maxShift, mOptimizeParameters.noiseStddev); });

      ImGui::BulletText("IPC accuracy measurement/optimization");
      if (ImGui::Button("Measure"))
        LaunchAsync([&]() { IPCMeasure::MeasureAccuracy(mIPC, mIPCOptimized, GetCurrentDatasetPath()); });
      ImGui::SameLine();
      if (ImGui::Button("Optimize"))
        LaunchAsync([&]() { IPCOptimization::Optimize(mIPCOptimized, GetCurrentDatasetPath(), mOptimizeParameters.popSize); });
      ImGui::InputText("image dir", &mOptimizeParameters.imageDirectory);
      ImGui::InputText("##generate dataset dir", &mOptimizeParameters.generateDirectory);
      ImGui::SameLine();
      if (ImGui::Button("Generate pairs"))
        LaunchAsync(
            [&]()
            {
              GenerateImageRegistrationDataset(mIPC, mOptimizeParameters.imageDirectory, mOptimizeParameters.generateDirectory, mOptimizeParameters.iters,
                  mOptimizeParameters.maxShift, mOptimizeParameters.noiseStddev);
            });
      ImGui::InputText("debug image1", &mOptimizeParameters.debugImage1Path);
      ImGui::InputText("debug image2", &mOptimizeParameters.debugImage2Path);
      ImGui::SliderFloat("max shift", &mOptimizeParameters.maxShift, 0.5, 3.0);
      ImGui::SliderFloat("noise stddev", &mOptimizeParameters.noiseStddev, 0.0, 0.1, nullptr, ImGuiSliderFlags_Logarithmic);
      ImGui::SliderInt("iters", &mOptimizeParameters.iters, 3, 201);
      ImGui::SliderFloat("test ratio", &mOptimizeParameters.testRatio, 0.0, 1.0);
      ImGui::SliderInt("popsize", &mOptimizeParameters.popSize, 6, 60);

      ImGui::BulletText("False correlations removal");
      if (ImGui::Button("IPC/FCR"))
        LaunchAsync([&]() { FalseCorrelationsRemoval(); });
    }

    ImGui::EndTabItem();
  }
}

void IPCWindow::UpdateIPCParameters(IPC& ipc)
{
  ipc.SetSize(mIPCParameters.Rows, mIPCParameters.Cols);
  ipc.SetBandpassParameters(mIPCParameters.BPL, mIPCParameters.BPH);
  ipc.SetL2size(mIPCParameters.L2size);
  ipc.SetL1ratio(mIPCParameters.L1ratio);
  ipc.SetL2Usize(mIPCParameters.L2Usize);
  ipc.SetMaxIterations(mIPCParameters.MaxIter);
  ipc.SetCrossPowerEpsilon(mIPCParameters.CPeps);
  ipc.SetWindowType(static_cast<IPC::WindowType>(mIPCParameters.WinT));
  ipc.SetBandpassType(static_cast<IPC::BandpassType>(mIPCParameters.BPT));
  ipc.SetInterpolationType(static_cast<IPC::InterpolationType>(mIPCParameters.IntT));
  ipc.SetL1WindowType(static_cast<IPC::L1WindowType>(mIPCParameters.L1WinT));
}

std::string IPCWindow::GetCurrentDatasetPath() const
{
  return fmt::format("../debug/ipcopt/imreg_dataset_{}x{}_{}i_{:.3f}ns", mIPC.GetCols(), mIPC.GetRows(), mOptimizeParameters.iters, mOptimizeParameters.noiseStddev);
}

void IPCWindow::FalseCorrelationsRemoval() const
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
