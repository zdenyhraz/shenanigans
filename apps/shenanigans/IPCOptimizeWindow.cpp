#include "IPCOptimizeWindow.hpp"
#include "IPCWindow.hpp"

void IPCOptimizeWindow::Initialize()
{
}

void IPCOptimizeWindow::Render()
{
  ImGui::Begin("IPC optimization");

  if (ImGui::Button("Optimize"))
    IPCOptimization::Optimize(
        IPCWindow::GetIPC(), mParameters.trainDirectory, mParameters.testDirectory, mParameters.maxShift, mParameters.noiseStddev, mParameters.iters, mParameters.testRatio, mParameters.popSize);

  ImGui::InputText("Train directory", &mParameters.trainDirectory);
  ImGui::InputText("Test directory", &mParameters.testDirectory);
  ImGui::SliderFloat("Max shift", &mParameters.maxShift, 0.5, 3.0);
  ImGui::SliderFloat("Noise stddev", &mParameters.noiseStddev, 0.0, 0.1);
  ImGui::SliderInt("Iters", &mParameters.iters, 1, 51);
  ImGui::SliderFloat("Test ratio", &mParameters.testRatio, 0.0, 1.0);
  ImGui::SliderInt("Population size", &mParameters.popSize, 6, 60);
  ImGui::End();
}
