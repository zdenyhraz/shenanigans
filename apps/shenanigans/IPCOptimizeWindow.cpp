#include "IPCOptimizeWindow.hpp"
#include "IPCWindow.hpp"

void IPCOptimizeWindow::Initialize()
{
  mImage = LoadUnitFloatImage<IPC::Float>(mParameters.imagePath);
}

void IPCOptimizeWindow::Render()
{
  ImGui::Begin("IPC optimization");

  if (ImGui::Button("Optimize"))
    LaunchAsync(
        []()
        {
          IPCOptimization::Optimize(
              IPCWindow::GetIPC(), mParameters.trainDirectory, mParameters.testDirectory, mParameters.maxShift, mParameters.noiseStddev, mParameters.iters, mParameters.testRatio, mParameters.popSize);
        });

  ImGui::SameLine();

  if (ImGui::Button("Accuracy map"))
    LaunchAsync([]() { IPCMeasure::MeasureAccuracyMap(IPCWindow::GetIPC(), mImage, mParameters.iters, mParameters.maxShift, mParameters.noiseStddev, &mProgress); });

  ImGui::ProgressBar(mProgress, ImVec2(0.f, 0.f));
  ImGui::InputText("##load path", &mParameters.imagePath);
  ImGui::SameLine();
  if (ImGui::Button("Load"))
    LaunchAsync(
        []()
        {
          mImage = LoadUnitFloatImage<IPC::Float>(mParameters.imagePath);
          LOG_DEBUG("Loaded image {}", mParameters.imagePath);
        });
  ImGui::InputText("train directory", &mParameters.trainDirectory);
  ImGui::InputText("test directory", &mParameters.testDirectory);
  ImGui::SliderFloat("max shift", &mParameters.maxShift, 0.5, 3.0);
  ImGui::SliderFloat("noise stddev", &mParameters.noiseStddev, 0.0, 0.5);
  ImGui::SliderInt("iters", &mParameters.iters, 1, 201);
  ImGui::SliderFloat("test ratio", &mParameters.testRatio, 0.0, 1.0);
  ImGui::SliderInt("popsize", &mParameters.popSize, 6, 60);
  ImGui::End();
}
