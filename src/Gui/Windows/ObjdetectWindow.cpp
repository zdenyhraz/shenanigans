#include "ObjdetectWindow.hpp"
#include "ML/ImageSegmentationModel.hpp"

void ObjdetectWindow::DetectObjects() const
{
  LOG_FUNCTION;
  auto image = LoadUnitFloatImage<f32>(GetProjectDirectoryPath() / mParameters.imagePath);
  if (mParameters.imageSize != 1)
    cv::resize(image, image, cv::Size(mParameters.imageSize * image.cols, mParameters.imageSize * image.rows));
  std::ignore = DetectObjectsSobelObjectness(image, mParameters.soParams);
}

void ObjdetectWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("Objdetect"))
  {
    ImGui::Separator();
    ImGui::BulletText("Object detection via Sobel objectness");
    if (ImGui::Button("Detect objects"))
      LaunchAsync([&]() { DetectObjects(); });
    ImGui::SameLine();
    if (ImGui::Button("Default"))
      LaunchAsync([&]() { mParameters = ObjdetectParameters(); });

    ImGui::Text("Input parameters");
    ImGui::InputText("image path", &mParameters.imagePath);
    ImGui::SliderFloat("image size", &mParameters.imageSizePercent, 10, 100, "%.0f%%");
    ImGui::SliderFloat("blur size", &mParameters.blurSizePercent, 0, 10, "%.0f%%");

    ImGui::Text("Edge detection parameters");
    ImGui::SliderFloat("edge size", &mParameters.edgeSizePercent, 0.1, 10, "%.2f%%", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("edge threshold", &mParameters.edgeThresholdPercent, 0, 50, "%.0f%%");

    ImGui::Text("Objectness parameters");
    ImGui::SliderFloat("object size", &mParameters.objectSizePercent, 1, 10, "%.1f%%");
    ImGui::SliderFloat("objectness threshold", &mParameters.objectnessThresholdPercent, 1, 50, "%.0f%%");

    ImGui::Text("Object filtering");
    ImGui::SliderFloat("min area", &mParameters.minObjectAreaPercent, 0, 5, "%.2f%%", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("max elongatedness", &mParameters.soParams.maxObjectElongatedness, 3, 30, "%.1f");

    ImGui::EndTabItem();
  }
  mParameters.Update();
}
