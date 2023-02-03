#include "DiffrotWindow.hpp"
#include "IPCWindow.hpp"

void DiffrotWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("Diffrot"))
  {
    ImGui::Separator();
    ImGui::BulletText("Solar differential rotation measurement");
    if (ImGui::Button("Calculate"))
      LaunchAsync(
          [&]()
          {
            mDiffrotData = DifferentialRotation::Calculate(mIPC, mParameters.dataPath, mParameters.xsize, mParameters.ysize, mParameters.idstep, mParameters.idstride,
                ToRadians(mParameters.thetamax), mParameters.cadence, mParameters.idstart, &mProgress);
          });

    ImGui::SameLine();
    if (ImGui::Button("Plot meridian curve"))
      LaunchAsync([&]() { DifferentialRotation::PlotMeridianCurve(mDiffrotData, mParameters.dataPath, 27); });

    if (ImGui::Button("Show IPC"))
      LaunchAsync(
          [&]()
          {
            const auto image1 =
                RoiCrop(LoadUnitFloatImage<IPC::Float>(fmt::format("{}/{}.png", mParameters.dataPath, mParameters.idstart)), 4096 / 2, 4096 / 2, mIPC.GetCols(), mIPC.GetRows());
            const auto image2 = RoiCrop(LoadUnitFloatImage<IPC::Float>(fmt::format("{}/{}.png", mParameters.dataPath, mParameters.idstart + mParameters.idstep)), 4096 / 2,
                4096 / 2, mIPC.GetCols(), mIPC.GetRows());

            mIPC.Calculate<IPC::Mode::Debug>(image1, image2);
          });

    ImGui::SameLine();
    if (ImGui::Button("Plot IPC gradual idstep"))
    {
      LaunchAsync([&]() { DifferentialRotation::PlotGradualIdStep(mIPC, 15); });
    }

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
      LaunchAsync([&]() { mDiffrotData.Load(mParameters.loadPath); });

    ImGui::Separator();

    if (ImGui::Button("Optimize"))
      LaunchAsync(
          [&]()
          {
            DifferentialRotation::Optimize(mIPC, mParameters.dataPath, mParameters.xsize, mParameters.ysize, mParameters.idstep, mParameters.idstride,
                ToRadians(mParameters.thetamax), mParameters.cadence, mParameters.idstart, mParameters.xsizeopt, mParameters.ysizeopt, mParameters.popsize);
          });

    ImGui::SliderInt("xsizeopt", &mParameters.xsizeopt, 1, 500);
    ImGui::SliderInt("ysizeopt", &mParameters.ysizeopt, 3, 201);
    ImGui::SliderInt("popsize", &mParameters.popsize, 6, 36);
    ImGui::EndTabItem();
  }
}
