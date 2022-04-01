#include "DiffrotWindow.hpp"
#include "IPCWindow.hpp"
#include "Astrophysics/DifferentialRotation.hpp"

void DiffrotWindow::Initialize()
{
}

void DiffrotWindow::Render()
{
  ImGui::Begin("Differential rotation");

  if (ImGui::Button("Calculate"))
    DifferentialRotation(mParameters.xsize, mParameters.ysize, mParameters.idstep, mParameters.idstride, mParameters.thetamax / Constants::Rad, mParameters.cadence)
        .Calculate(IPCWindow::GetIPC(), mParameters.dataPath, mParameters.idstart);

  ImGui::SliderInt("xsize", &mParameters.xsize, 1, 2500);
  ImGui::SliderInt("ysize", &mParameters.ysize, 3, 301);
  ImGui::SliderInt("id step", &mParameters.idstep, 1, 5);
  ImGui::SliderInt("id stride", &mParameters.idstride, 1, 25);
  ImGui::SliderFloat("thetamax", &mParameters.thetamax, 20, 80);
  ImGui::SliderInt("cadence", &mParameters.cadence, 25, 100);
  ImGui::InputText("Data path", &mParameters.dataPath);
  ImGui::InputInt("id start", &mParameters.idstart);

  if (ImGui::Button("Optimize"))
    DifferentialRotation(mParameters.xsize, mParameters.ysize, mParameters.idstep, mParameters.idstride, mParameters.thetamax / Constants::Rad, mParameters.cadence)
        .Optimize(IPCWindow::GetIPC(), mParameters.dataPath, mParameters.idstart, mParameters.xsizeopt, mParameters.ysizeopt, mParameters.popsize);

  ImGui::SliderInt("xsizeopt", &mParameters.xsizeopt, 1, 500);
  ImGui::SliderInt("ysizeopt", &mParameters.ysizeopt, 3, 201);
  ImGui::SliderInt("popsize", &mParameters.popsize, 6, 36);

  ImGui::End();
}
