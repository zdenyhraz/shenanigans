#include "SwindWindow.hpp"

void SwindWindow::Initialize()
{
}

void SwindWindow::Render()
{
  if (ImGui::BeginTabItem("Swind"))
  {
    ImGui::Separator();

    if (ImGui::Button("Calculate"))
      LaunchAsync([]() { featureMatch(mParameters); });

    ImGui::SliderFloat("thresh", &mParameters.thresh, 0, 500);
    ImGui::SliderInt("matchcnt", &mParameters.matchcnt, 100, 2000);
    ImGui::SliderFloat("minSpeed", &mParameters.minSpeed, 0, 1000);
    ImGui::SliderFloat("maxSpeed", &mParameters.maxSpeed, 500, 1500);
    ImGui::InputText("path", &mParameters.path);
    ImGui::SliderFloat("overlapdistance", &mParameters.overlapdistance, 0, 200);
    ImGui::SliderFloat("ratioThreshold", &mParameters.ratioThreshold, 0, 1);
    ImGui::SliderFloat("upscale", &mParameters.upscale, 1, 5);
    ImGui::SliderInt("nOctaves", &mParameters.nOctaves, 1, 10);
    ImGui::SliderInt("nOctaveLayers", &mParameters.nOctaveLayers, 1, 10);
    ImGui::Checkbox("surfExtended", &mParameters.surfExtended);
    ImGui::SameLine();
    ImGui::Checkbox("surfUpright", &mParameters.surfUpright);
    ImGui::Checkbox("mask", &mParameters.mask);
    ImGui::SameLine();
    ImGui::Checkbox("drawOverlapCircles", &mParameters.drawOverlapCircles);

    ImGui::EndTabItem();
  }
}
