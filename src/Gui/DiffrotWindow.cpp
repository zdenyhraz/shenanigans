#include "DiffrotWindow.hpp"
#include "IPCWindow.hpp"

void DiffrotWindow::Initialize()
{
}

void DiffrotWindow::Render()
{
  ImGui::Begin("Differential rotation");

  if (ImGui::Button("Calculate"))
    mDiffrotData = DifferentialRotation::Calculate(IPCWindow::GetIPC(), mParameters.dataPath, mParameters.xsize, mParameters.ysize, mParameters.idstep, mParameters.idstride,
        mParameters.thetamax / Rad, mParameters.cadence, mParameters.idstart, &mProgress);
  ImGui::SameLine();
  if (ImGui::Button("Plot meridian curve"))
    DifferentialRotation::PlotMeridianCurve(mDiffrotData, mParameters.dataPath, 27);

  ImGui::ProgressBar(mProgress, ImVec2(0.f, 0.f));
  ImGui::SliderInt("xsize", &mParameters.xsize, 1, 2500);
  ImGui::SliderInt("ysize", &mParameters.ysize, 3, 301);
  ImGui::SliderInt("id step", &mParameters.idstep, 1, 5);
  ImGui::SliderInt("id stride", &mParameters.idstride, 1, 25);
  ImGui::SliderFloat("thetamax", &mParameters.thetamax, 20, 80);
  ImGui::SliderInt("cadence", &mParameters.cadence, 25, 100);
  ImGui::InputInt("id start", &mParameters.idstart);
  ImGui::InputText("data path", &mParameters.dataPath);
  ImGui::InputText("##load path", &mParameters.loadPath);
  ImGui::SameLine();
  if (ImGui::Button("Load"))
    mDiffrotData.Load(mParameters.loadPath);

  ImGui::Separator();

  if (ImGui::Button("Optimize"))
    DifferentialRotation::Optimize(IPCWindow::GetIPC(), mParameters.dataPath, mParameters.xsize, mParameters.ysize, mParameters.idstep, mParameters.idstride, mParameters.thetamax / Rad,
        mParameters.cadence, mParameters.idstart, mParameters.xsizeopt, mParameters.ysizeopt, mParameters.popsize);
  ImGui::SliderInt("xsizeopt", &mParameters.xsizeopt, 1, 500);
  ImGui::SliderInt("ysizeopt", &mParameters.ysizeopt, 3, 201);
  ImGui::SliderInt("popsize", &mParameters.popsize, 6, 36);

  ImGui::End();
}
