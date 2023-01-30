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
    ImGui::InputText("image path", &mParameters.imagePath);
    ImGui::SliderFloat("imageSizeMultiplier", &mParameters.imageSizeMultiplier, 0.1, 1);
    ImGui::SliderFloat("blurSizeMultiplier", &mParameters.soParams.blurSizeMultiplier, 0, 0.1);

    ImGui::Text("Edge detection parameters");
    ImGui::SliderFloat("edgeSizeMultiplier", &mParameters.soParams.edgeSizeMultiplier, 0.001, 0.01);
    ImGui::SliderFloat("edgeThreshold", &mParameters.soParams.edgeThreshold, 0.01, 0.5);

    ImGui::Text("Objectness parameters");
    ImGui::SliderFloat("objectSizeMultiplier", &mParameters.soParams.objectSizeMultiplier, 0.001, 0.1);
    ImGui::SliderFloat("objectnessThreshold", &mParameters.soParams.objectnessThreshold, 0.001, 0.3);

    ImGui::Text("Object filtering");
    ImGui::SliderFloat("minObjectSizeMultiplier", &mParameters.soParams.minObjectSizeMultiplier, 0, 0.1);
    ImGui::EndTabItem();
  }
}
