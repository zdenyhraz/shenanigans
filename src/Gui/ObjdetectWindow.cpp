#include "ObjdetectWindow.hpp"

void ObjdetectWindow::DetectObjects() const
{
  LOG_FUNCTION;
  auto image = LoadUnitFloatImage<f32>(GetProjectDirectoryPath() / mParameters.imagePath);
  if (mParameters.imageSizeMultiplier != 1)
    cv::resize(image, image, cv::Size(mParameters.imageSizeMultiplier * image.cols, mParameters.imageSizeMultiplier * image.rows));
  std::ignore = DetectObjectsSobelObjectness(image, mParameters.soParams);
}

void ObjdetectWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("Objdetect"))
  {
    ImGui::Separator();
    ImGui::Text("Object detection via Sobel objectness");
    if (ImGui::Button("DetectObjects"))
      LaunchAsync([&]() { DetectObjects(); });

    ImGui::Text("Input parameters");
    ImGui::SameLine();
    if (ImGui::Button("Default"))
      LaunchAsync([&]() { mParameters = ObjdetectParameters(); });
    ImGui::InputText("image path", &mParameters.imagePath);
    ImGui::SliderFloat("imageSize", &mParameters.imageSizeMultiplier, 0.1, 1);
    ImGui::SliderFloat("blurSize", &mParameters.soParams.blurSizeMultiplier, 0, 0.1);

    ImGui::Text("Edge detection parameters");
    ImGui::SliderFloat("edgeSize", &mParameters.soParams.edgeSizeMultiplier, 0.001, 0.1, "%.4f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("edgeThreshold", &mParameters.soParams.edgeThreshold, 0.0, 0.5, "%.4f");

    ImGui::Text("Objectness parameters");
    ImGui::SliderFloat("objectSize", &mParameters.soParams.objectSizeMultiplier, 0.001, 0.1, "%.4f");
    ImGui::SliderFloat("objectnessThreshold", &mParameters.soParams.objectnessThreshold, 0.001, 0.5, "%.4f");

    ImGui::Text("Object filtering");
    ImGui::SliderFloat("minObjectArea", &mParameters.soParams.minObjectAreaMultiplier, 0, 0.05, "%.4f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("minObjectWidth", &mParameters.soParams.minObjectWidthMultiplier, 0, 0.1, "%.4f");
    ImGui::EndTabItem();
  }
}
