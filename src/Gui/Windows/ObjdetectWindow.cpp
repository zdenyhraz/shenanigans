#include "ObjdetectWindow.hpp"
#include "DNN/ObjectDetection/YOLOv3CV.hpp"
#include "DNN/ObjectDetection/YOLOv8CV.hpp"
#include "DNN/ObjectDetection/YOLOv8Torch.hpp"

void ObjdetectWindow::DetectObjectsSO() const
{
  LOG_FUNCTION;
  auto image = LoadUnitFloatImage<f32>(GetProjectDirectoryPath(imagePath));
  if (mSOParameters.imageSize != 1)
    cv::resize(image, image, cv::Size(mSOParameters.imageSize * image.cols, mSOParameters.imageSize * image.rows));
  std::ignore = DetectObjectsSobelObjectness(image, mSOParameters.soParams);
}

void ObjdetectWindow::DetectObjectsYOLOv3CVW() const
{
  LOG_FUNCTION;
  const auto image = LoadImage(GetProjectDirectoryPath(imagePath));
  DetectObjectsYOLOv3CV(
      image, GetProjectDirectoryPath(modelPath + ".weights"), GetProjectDirectoryPath(modelPath + ".cfg"), "Darknet", GetProjectDirectoryPath(classesPath), confidenceThreshold);
}

void ObjdetectWindow::DetectObjectsYOLOv8CVW() const
{
  LOG_FUNCTION;
  const auto image = LoadImage(GetProjectDirectoryPath(imagePath));
  DetectObjectsYOLOv8CV(image, GetProjectDirectoryPath(modelPath + ".onnx"), GetProjectDirectoryPath(classesPath), confidenceThreshold, NMSThreshold);
}

void ObjdetectWindow::DetectObjectsYOLOv8TorchW() const
{
  LOG_FUNCTION;
  const auto image = LoadImage(GetProjectDirectoryPath(imagePath));
  DetectObjectsYOLOv8Torch(image, GetProjectDirectoryPath(modelPath + ".torchscript"), GetProjectDirectoryPath(classesPath), confidenceThreshold, NMSThreshold);
}

void ObjdetectWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("Objdetect"))
  {
    ImGui::Separator();
    ImGui::InputText("image path", &imagePath);

    // ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Object detection via Sobel objectness"))
    {
      if (ImGui::Button("Detect objects"))
        LaunchAsync([&]() { DetectObjectsSO(); });
      ImGui::SameLine();
      if (ImGui::Button("Default"))
        LaunchAsync([&]() { mSOParameters = SobelObjectnessWindowParameters(); });

      ImGui::Text("Input parameters");
      ImGui::SliderFloat("image size", &mSOParameters.imageSizePercent, 10, 100, "%.0f%%");
      ImGui::SliderFloat("blur size", &mSOParameters.blurSizePercent, 0, 10, "%.0f%%");

      ImGui::Text("Edge detection parameters");
      ImGui::SliderFloat("edge size", &mSOParameters.edgeSizePercent, 0.1, 10, "%.2f%%", ImGuiSliderFlags_Logarithmic);
      ImGui::SliderFloat("edge threshold", &mSOParameters.edgeThresholdPercent, 0, 50, "%.0f%%");

      ImGui::Text("Objectness parameters");
      ImGui::SliderFloat("object size", &mSOParameters.objectSizePercent, 1, 10, "%.1f%%");
      ImGui::SliderFloat("objectness threshold", &mSOParameters.objectnessThresholdPercent, 1, 50, "%.0f%%");

      ImGui::Text("Object filtering");
      ImGui::SliderFloat("min area", &mSOParameters.minObjectAreaPercent, 0, 5, "%.2f%%", ImGuiSliderFlags_Logarithmic);
      ImGui::SliderFloat("max elongatedness", &mSOParameters.soParams.maxObjectElongatedness, 3, 30, "%.1f");
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Object detection via DNN"))
    {
      ImGui::InputText("model path", &modelPath);
      ImGui::InputText("classes path", &classesPath);
      ImGui::SliderFloat("confidence threshold", &confidenceThreshold, 0, 1, "%.2f");
      ImGui::SliderFloat("NMS threshold", &NMSThreshold, 0, 1, "%.2f");

      ImGui::Text("YOLOv3");
      if (ImGui::Button("YOLOv3 OpenCV"))
        LaunchAsync([&]() { DetectObjectsYOLOv3CVW(); });
      ImGui::Text("YOLOv8");
      if (ImGui::Button("YOLOv8 OpenCV"))
        LaunchAsync([&]() { DetectObjectsYOLOv8CVW(); });
      ImGui::SameLine();
      if (ImGui::Button("YOLOv8 Torch"))
        LaunchAsync([&]() { DetectObjectsYOLOv8TorchW(); });
    }

    ImGui::EndTabItem();
  }
  mSOParameters.Update();
}
