#include "AstroWindow.hpp"
#include "Utils/Async.hpp"

void AstroWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("Astro"))
  {
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Solar differential rotation measurement"))
    {
      if (ImGui::Button("Calculate"))
        LaunchAsync(
            [&]()
            {
              mDiffrotData = DifferentialRotation::Calculate(mIPC, mDiffrotParameters.dataPath, mDiffrotParameters.xsize, mDiffrotParameters.ysize, mDiffrotParameters.idstep,
                  mDiffrotParameters.idstride, ToRadians(mDiffrotParameters.thetamax), mDiffrotParameters.cadence, mDiffrotParameters.idstart, &mProgress);
            });

      ImGui::SameLine();
      if (ImGui::Button("Plot meridian curve"))
        LaunchAsync([&]() { DifferentialRotation::PlotMeridianCurve(mDiffrotData, mDiffrotParameters.dataPath, 27); });

      if (ImGui::Button("Show IPC"))
        LaunchAsync(
            [&]()
            {
              const auto image1 = RoiCrop(LoadUnitFloatImage<IPC::Float>(fmt::format("{}/{}.png", mDiffrotParameters.dataPath, mDiffrotParameters.idstart)), 4096 / 2, 4096 / 2,
                  mIPC.GetCols(), mIPC.GetRows());
              const auto image2 =
                  RoiCrop(LoadUnitFloatImage<IPC::Float>(fmt::format("{}/{}.png", mDiffrotParameters.dataPath, mDiffrotParameters.idstart + mDiffrotParameters.idstep)), 4096 / 2,
                      4096 / 2, mIPC.GetCols(), mIPC.GetRows());

              mIPC.Calculate<IPC::Mode::Debug>(image1, image2);
            });

      ImGui::SameLine();
      if (ImGui::Button("Plot IPC gradual idstep"))
      {
        LaunchAsync([&]() { DifferentialRotation::PlotGradualIdStep(mIPC, 15); });
      }

      ImGui::ProgressBar(mProgress, ImVec2(0.f, 0.f));
      ImGui::SliderInt("xsize", &mDiffrotParameters.xsize, 1, 2500);
      ImGui::SliderInt("ysize", &mDiffrotParameters.ysize, 3, 301);
      ImGui::SliderInt("id step", &mDiffrotParameters.idstep, 1, 5);
      ImGui::SliderInt("id stride", &mDiffrotParameters.idstride, 1, 25);
      ImGui::SliderFloat("thetamax", &mDiffrotParameters.thetamax, 20, 80);
      ImGui::SliderInt("cadence", &mDiffrotParameters.cadence, 25, 100);
      ImGui::InputInt("id start", &mDiffrotParameters.idstart);
      ImGui::InputText("data path", &mDiffrotParameters.dataPath);
      ImGui::InputText("##load path", &mDiffrotParameters.loadPath);
      ImGui::SameLine();
      if (ImGui::Button("Load"))
        LaunchAsync([&]() { mDiffrotData.Load(mDiffrotParameters.loadPath); });

      ImGui::Separator();

      if (ImGui::Button("Optimize"))
        LaunchAsync(
            [&]()
            {
              DifferentialRotation::Optimize(mIPC, mDiffrotParameters.dataPath, mDiffrotParameters.xsize, mDiffrotParameters.ysize, mDiffrotParameters.idstep,
                  mDiffrotParameters.idstride, ToRadians(mDiffrotParameters.thetamax), mDiffrotParameters.cadence, mDiffrotParameters.idstart, mDiffrotParameters.xsizeopt,
                  mDiffrotParameters.ysizeopt, mDiffrotParameters.popsize);
            });

      ImGui::SliderInt("xsizeopt", &mDiffrotParameters.xsizeopt, 1, 500);
      ImGui::SliderInt("ysizeopt", &mDiffrotParameters.ysizeopt, 3, 201);
      ImGui::SliderInt("popsize", &mDiffrotParameters.popsize, 6, 36);
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Solar wind speed measurement"))
    {
      if (ImGui::Button("Calculate"))
        LaunchAsync([&]() { MeasureSolarWindSpeed(mSwindParameters); });

      ImGui::SliderFloat("thresh", &mSwindParameters.thresh, 0, 500);
      ImGui::SliderInt("matchcnt", &mSwindParameters.matchcnt, 100, 2000);
      ImGui::SliderFloat("minSpeed", &mSwindParameters.minSpeed, 0, 1000);
      ImGui::SliderFloat("maxSpeed", &mSwindParameters.maxSpeed, 500, 1500);
      ImGui::InputText("path", &mSwindParameters.path);
      ImGui::SliderFloat("overlapdistance", &mSwindParameters.overlapdistance, 0, 200);
      ImGui::SliderFloat("ratioThreshold", &mSwindParameters.ratioThreshold, 0, 1);
      ImGui::SliderFloat("upscale", &mSwindParameters.upscale, 1, 5);
      ImGui::SliderInt("nOctaves", &mSwindParameters.nOctaves, 1, 10);
      ImGui::SliderInt("nOctaveLayers", &mSwindParameters.nOctaveLayers, 1, 10);
      ImGui::Checkbox("surfExtended", &mSwindParameters.surfExtended);
      ImGui::SameLine();
      ImGui::Checkbox("surfUpright", &mSwindParameters.surfUpright);
      ImGui::Checkbox("mask", &mSwindParameters.mask);
      ImGui::SameLine();
      ImGui::Checkbox("drawOverlapCircles", &mSwindParameters.drawOverlapCircles);
    }

    ImGui::EndTabItem();
  }
}
