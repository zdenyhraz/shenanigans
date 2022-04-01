#include "IPCMeasureWindow.hpp"
#include "IPCWindow.hpp"

void IPCMeasureWindow::Initialize()
{
  mImage = LoadUnitFloatImage<IPC::Float>(mParameters.imagePath);
}

void IPCMeasureWindow::Render()
{
  ImGui::Begin("IPC measure");
  if (ImGui::Button("Accuracy map"))
    IPCMeasure::MeasureAccuracyMap(IPCWindow::GetIPC(), mImage, mParameters.iters);

  ImGui::SliderInt("iters", &mParameters.iters, 5, 201);
  ImGui::InputText("##load path", &mParameters.imagePath);
  ImGui::SameLine();
  if (ImGui::Button("Load"))
    mImage = LoadUnitFloatImage<IPC::Float>(mParameters.imagePath);

  ImGui::End();
}
