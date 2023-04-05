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

SobelObjectnessParameters ObjdetectWindow::OptimizeSOParameters() const
{
  const auto image = LoadUnitFloatImage<f32>(imagePath.starts_with("data/") ? GetProjectDirectoryPath(imagePath) : std::filesystem::path(imagePath));
  const auto objects = DetectObjectsSobelObjectness(image, SobelObjectnessParameters());
  return OptimizeSobelObjectnessParameters({{image, objects}});
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
    ImGui::InputText("image path", &imagePath);
    ImGui::InputText("image directory path", &imageDirectoryPath);

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Object detection via Sobel objectness"))
    {
      if (ImGui::Button("Default"))
        LaunchAsync([&]() { mSOParameters = SobelObjectnessParameters(); });
      ImGui::SameLine();
      if (ImGui::Button("Optimize"))
        LaunchAsync([&]() { mSOParameters = OptimizeSOParameters(); });
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

      ImGui::Text("Input parameters");
      ImGui::SliderInt("image size", &mSOParameters.imageSize, 256, 4096);
      ImGui::SliderFloat("relative blur size", &mSOParameters.blurSize, 0, 0.1, "%.3f");

      ImGui::Text("Edge detection parameters");
      ImGui::SliderInt("edge size", &mSOParameters.edgeSize, 3, 31);

      ImGui::Text("Objectness parameters");
      ImGui::SliderFloat("relative objectness radius", &mSOParameters.objectnessRadius, 0.01, 0.1, "%.3f");
      ImGui::SliderFloat("objectness threshold", &mSOParameters.objectnessThreshold, 0, 1, "%.2f");

      ImGui::Text("Object filtering");
      ImGui::Checkbox("draw filtered", &mSOParameters.drawFiltered);
      ImGui::SameLine();
      ImGui::Checkbox("draw bboxes", &mSOParameters.drawBboxes);
      ImGui::SameLine();
      ImGui::Checkbox("draw contours", &mSOParameters.drawContours);
      ImGui::SliderFloat("relative min area", &mSOParameters.minObjectArea, 0, 0.05, "%.3f", ImGuiSliderFlags_Logarithmic);
      ImGui::SliderFloat("max elongatedness", &mSOParameters.maxObjectElongatedness, 3, 30, "%.1f");
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Object detection via DNN"))
    {
      ImGui::InputText("model path", &modelPath);
      ImGui::InputText("classes path", &classesPath);
      ImGui::SliderFloat("confidence threshold", &confidenceThreshold, 0, 1, "%.2f");
      ImGui::SliderFloat("NMS threshold", &NMSThreshold, 0, 1, "%.2f");

      ImGui::Text("YOLOv8");
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
