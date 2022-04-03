#include "IPCOptimizeWindow.hpp"
#include "IPCWindow.hpp"

void IPCOptimizeWindow::Initialize()
{
}

void IPCOptimizeWindow::Render()
{
  ImGui::Begin("IPC optimization");

  if (ImGui::Button("Optimize"))
    LaunchAsync(
        []()
        {
          IPCOptimization::Optimize(IPCWindow::GetIPC(), mParameters.trainDirectory, mParameters.testDirectory, mParameters.maxShift, mParameters.noiseStddev, mParameters.optiters,
              mParameters.testRatio, mParameters.popSize);
        });

  ImGui::SameLine();

  if (ImGui::Button("Accuracy map"))
    LaunchAsync([]() { IPCMeasure::MeasureAccuracy(IPCWindow::GetIPC(), mParameters.testDirectory, mParameters.iters, mParameters.maxShift, mParameters.noiseStddev, &mProgress); });

  ImGui::ProgressBar(mProgress, ImVec2(0.f, 0.f));
  ImGui::InputText("train directory", &mParameters.trainDirectory);
  ImGui::InputText("test directory", &mParameters.testDirectory);
  ImGui::SliderFloat("max shift", &mParameters.maxShift, 0.5, 3.0);
  ImGui::SliderFloat("noise stddev", &mParameters.noiseStddev, 0.0, 0.5);
  ImGui::SliderInt("iters", &mParameters.iters, 3, 501);
  ImGui::SliderInt("opt iters", &mParameters.optiters, 3, 201);
  ImGui::SliderFloat("test ratio", &mParameters.testRatio, 0.0, 1.0);
  ImGui::SliderInt("popsize", &mParameters.popSize, 6, 60);
  ImGui::End();
}
