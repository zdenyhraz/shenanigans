#include "ObjdetectWindow.hpp"

void ObjdetectWindow::DetectObjects()
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
    ImGui::InputText("image path", &mParameters.imagePath);
    ImGui::SliderFloat("imageSizeMultiplier", &mParameters.imageSizeMultiplier, 0.1, 1);
    ImGui::Separator();
    ImGui::Text("Object detection via Sobel objectness");
    if (ImGui::Button("DetectObjects"))
      LaunchAsync([&]() { DetectObjects(); });
    ImGui::SliderFloat("objectSizeMultiplier", &mParameters.soParams.objectSizeMultiplier, 0.001, 0.3);
    ImGui::SliderFloat("blurSizeMultiplier", &mParameters.soParams.blurSizeMultiplier, 0, 0.3);
    ImGui::SliderFloat("edgeSizeMultiplier", &mParameters.soParams.edgeSizeMultiplier, 0.001, 0.1);
    ImGui::SliderFloat("edgeThreshold", &mParameters.soParams.edgeThreshold, 0.01, 1);
    ImGui::SliderFloat("objectnessThreshold", &mParameters.soParams.objectnessThreshold, 0.001, 1);
    ImGui::SliderFloat("minObjectSizeMultiplier", &mParameters.soParams.minObjectSizeMultiplier, 0, 0.2);
    ImGui::EndTabItem();
  }
}
