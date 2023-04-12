#include "ObjdetectWindow.hpp"
#include "DNN/ObjectDetection/YOLOv8CV.hpp"
#include "DNN/ObjectDetection/YOLOv8Torch.hpp"
#include "DNN/ObjectSegmentation/YOLOv8Torch.hpp"

void ObjdetectWindow::DetectObjectsSO() const
{
  LOG_FUNCTION;
  const auto image = LoadUnitFloatImage<f32>(imagePath.starts_with("data/") ? GetProjectDirectoryPath(imagePath) : std::filesystem::path(imagePath));
  std::ignore = DetectObjectsSobelObjectness(image, mSOParameters);
}

void ObjdetectWindow::DetectObjectsSODirectory() const
{
  LOG_FUNCTION;
  const auto fileCount = GetFileCount(GetProjectDirectoryPath(imageDirectoryPath));
  for (const auto& entry : std::filesystem::directory_iterator(GetProjectDirectoryPath(imageDirectoryPath)))
  {
    static usize fileIndex = 0;
    LOG_PROGRESS_NAME(entry.path().string());
    LOG_PROGRESS(static_cast<f32>(++fileIndex) / fileCount);
    const auto image = LoadUnitFloatImage<f32>(entry.path());
    std::ignore = DetectObjectsSobelObjectness(image, mSOParameters);
  }
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

void ObjdetectWindow::DetectObjectsYOLOv8SegTorchW() const
{
  LOG_FUNCTION;
  const auto image = LoadImage(GetProjectDirectoryPath(imagePath));
  SegmentObjectsYOLOv8Torch(image, GetProjectDirectoryPath(modelPath + "-seg.torchscript"), GetProjectDirectoryPath(classesPath), confidenceThreshold, NMSThreshold);
}

void ObjdetectWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("Objdetect"))
  {
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Object detection via Sobel objectness"))
    {
      ImGui::BulletText("Image source");
      ImGui::SameLine();
      if (ImGui::Button("Detect objects"))
        LaunchAsync([&]() { DetectObjectsSO(); });
      ImGui::SameLine();
      if (ImGui::Button("Detect objects next"))
        LaunchAsync(
            [&]()
            {
              imagePath = filePathGenerator.GetNextFilePath().string();
              if (imagePath.empty())
              {
                filePathGenerator.Reset();
                imagePath = filePathGenerator.GetNextFilePath().string();
              }
              DetectObjectsSO();
            });
      ImGui::SameLine();
      if (ImGui::Button("Detect objects dir"))
        LaunchAsync([&]() { DetectObjectsSODirectory(); });
      ImGui::InputText("image", &imagePath);
      ImGui::InputText("image directory", &imageDirectoryPath);

      ImGui::BulletText("Parameter optimization");
      ImGui::SameLine();
      if (ImGui::Button("Optimize"))
        LaunchAsync([&]()
            { mSOParameters = OptimizeSobelObjectnessParameters(GetProjectDirectoryPath(optimizeImageDirectoryPath), GetProjectDirectoryPath(optimizeObjectDirectoryPath)); });
      ImGui::InputText("optimize image directory", &optimizeImageDirectoryPath);
      ImGui::InputText("optimize object directory", &optimizeObjectDirectoryPath);

      ImGui::BulletText("Parameters");
      ImGui::SameLine();
      if (ImGui::Button("Default"))
        LaunchAsync([&]() { mSOParameters = SobelObjectnessParameters(); });
      ImGui::SliderInt("image size", &mSOParameters.imageSize, 256, 4096);
      ImGui::SliderFloat("relative blur size", &mSOParameters.blurSize, 0, 0.1, "%.3f");
      ImGui::SliderFloat("relative edge size", &mSOParameters.edgeSize, 0.001, 0.1, "%.3f", ImGuiSliderFlags_Logarithmic);
      ImGui::SliderFloat("relative objectness radius", &mSOParameters.objectnessRadius, 0.01, 0.1, "%.3f");
      ImGui::SliderFloat("objectness threshold", &mSOParameters.objectnessThreshold, 0, 0.3, "%.3f", ImGuiSliderFlags_Logarithmic);
      ImGui::SliderFloat("shadow threshold", &mSOParameters.shadowThreshold, 0, 1, "%.3f", ImGuiSliderFlags_Logarithmic);
      ImGui::SliderFloat("relative min area", &mSOParameters.minObjectArea, 0, 0.1, "%.3f");
      ImGui::SliderFloat("max elongatedness", &mSOParameters.maxObjectElongatedness, 3, 30, "%.1f");
      ImGui::Checkbox("draw filtered", &mSOParameters.drawFiltered);
      ImGui::SameLine();
      ImGui::Checkbox("draw bboxes", &mSOParameters.drawBboxes);
      ImGui::Checkbox("draw contours", &mSOParameters.drawContours);
      ImGui::SameLine();
      ImGui::Checkbox("draw shadow contours", &mSOParameters.drawShadowContours);
    }

    // ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Object detection via DNN"))
    {
      ImGui::InputText("model path", &modelPath);
      ImGui::InputText("classes path", &classesPath);
      ImGui::SliderFloat("confidence threshold", &confidenceThreshold, 0, 1, "%.2f");
      ImGui::SliderFloat("NMS threshold", &NMSThreshold, 0, 1, "%.2f");

      ImGui::BulletText("YOLOv8");
      if (ImGui::Button("YOLOv8 OpenCV"))
        LaunchAsync([&]() { DetectObjectsYOLOv8CVW(); });
      ImGui::SameLine();
      if (ImGui::Button("YOLOv8 Torch"))
        LaunchAsync([&]() { DetectObjectsYOLOv8TorchW(); });
      ImGui::SameLine();
      if (ImGui::Button("YOLOv8-seg Torch"))
        LaunchAsync([&]() { DetectObjectsYOLOv8SegTorchW(); });
    }

    ImGui::EndTabItem();
  }
}
