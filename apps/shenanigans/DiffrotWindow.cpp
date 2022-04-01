#include "DiffrotWindow.hpp"
#include "IPCWindow.hpp"

void DiffrotWindow::Initialize()
{
}

void DiffrotWindow::Render()
{
  ImGui::Begin("Differential rotation");

  if (ImGui::Button("Calculate"))
    mDiffrotData = GetDiffrot().Calculate(IPCWindow::GetIPC(), mParameters.dataPath, mParameters.idstart);
  ImGui::SameLine();
  if (ImGui::Button("Plot meridian curve"))
    DifferentialRotation::PlotMeridianCurve(mDiffrotData, mParameters.dataPath, mParameters.idstart, 27);

  ImGui::SliderInt("xsize", &mParameters.xsize, 1, 2500);
  ImGui::SliderInt("ysize", &mParameters.ysize, 3, 301);
  ImGui::SliderInt("id step", &mParameters.idstep, 1, 5);
  ImGui::SliderInt("id stride", &mParameters.idstride, 1, 25);
  ImGui::SliderFloat("thetamax", &mParameters.thetamax, 20, 80);
  ImGui::SliderInt("cadence", &mParameters.cadence, 25, 100);
  ImGui::InputText("data path", &mParameters.dataPath);
  ImGui::InputInt("id start", &mParameters.idstart);
  ImGui::InputText("##load path", &mParameters.loadPath);
  ImGui::SameLine();
  if (ImGui::Button("Load"))
    mDiffrotData.Load(mParameters.loadPath);
  ImGui::SameLine();
  if (ImGui::Button("Show"))
    mDiffrotData.Load(mParameters.loadPath);

  if (ImGui::Button("Optimize"))
    GetDiffrot().Optimize(IPCWindow::GetIPC(), mParameters.dataPath, mParameters.idstart, mParameters.xsizeopt, mParameters.ysizeopt, mParameters.popsize);
  ImGui::SliderInt("xsizeopt", &mParameters.xsizeopt, 1, 500);
  ImGui::SliderInt("ysizeopt", &mParameters.ysizeopt, 3, 201);
  ImGui::SliderInt("popsize", &mParameters.popsize, 6, 36);

  ImGui::End();
}

DifferentialRotation DiffrotWindow::GetDiffrot()
{
  return DifferentialRotation(mParameters.xsize, mParameters.ysize, mParameters.idstep, mParameters.idstride, mParameters.thetamax / Constants::Rad, mParameters.cadence);
}
