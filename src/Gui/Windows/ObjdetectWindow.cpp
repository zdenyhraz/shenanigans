#include "ObjdetectWindow.hpp"
#include "DNN/ObjectDetection/YOLOv8CV.hpp"
#include "DNN/ObjectDetection/YOLOv8Torch.hpp"
#include "DNN/ObjectSegmentation/YOLOv8Torch.hpp"

void ObjdetectWindow::DetectObjectsSO() const
{
  LOG_FUNCTION;

  auto image = LoadUnitFloatImage<f32>(imagePath.starts_with("data/") ? GetProjectDirectoryPath(imagePath) : std::filesystem::path(imagePath));
  if (mSOParameters.imageSize != 1)
    cv::resize(image, image, cv::Size(mSOParameters.imageSize * image.cols, mSOParameters.imageSize * image.rows));
  std::ignore = DetectObjectsSobelObjectness(image, mSOParameters.soParams);
}

SobelObjectnessParameters ObjdetectWindow::OptimizeSOParameters() const
{
  auto image = LoadUnitFloatImage<f32>(imagePath.starts_with("data/") ? GetProjectDirectoryPath(imagePath) : std::filesystem::path(imagePath));
  if (mSOParameters.imageSize != 1)
    cv::resize(image, image, cv::Size(mSOParameters.imageSize * image.cols, mSOParameters.imageSize * image.rows));
  const auto objects = DetectObjectsSobelObjectness(image, SobelObjectnessParameters());
  return OptimizeSobelObjectnessParameters({{image, objects}});
}

void ObjdetectWindow::DetectObjectsSODirectory() const
{
  LOG_FUNCTION;
  if (not std::filesystem::exists(GetProjectDirectoryPath(saveDirectoryPath)))
    std::filesystem::create_directory(GetProjectDirectoryPath(saveDirectoryPath));
  const auto fileCount = GetFileCount(GetProjectDirectoryPath(imageDirectoryPath));
  for (const auto& entry : std::filesystem::directory_iterator(GetProjectDirectoryPath(imageDirectoryPath)))
  {
    static usize fileIndex = 0;
    LOG_PROGRESS_NAME(entry.path().string());
    LOG_PROGRESS(static_cast<f32>(++fileIndex) / fileCount);

    auto image = LoadUnitFloatImage<f32>(entry.path());
    if (mSOParameters.imageSize != 1)
      cv::resize(image, image, cv::Size(mSOParameters.imageSize * image.cols, mSOParameters.imageSize * image.rows));
    std::ignore = DetectObjectsSobelObjectness(image, mSOParameters.soParams, GetProjectDirectoryPath(saveDirectoryPath) / entry.path().filename());
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
    ImGui::InputText("save directory path", &saveDirectoryPath);

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Object detection via Sobel objectness"))
    {
      if (ImGui::Button("Default"))
        LaunchAsync([&]() { mSOParameters = SobelObjectnessWindowParameters(); });
      ImGui::SameLine();
      if (ImGui::Button("Optimize"))
        LaunchAsync([&]() { mSOParameters.soParams = OptimizeSOParameters(); });
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
      ImGui::SliderFloat("image size", &mSOParameters.imageSizePercent, 10, 100, "%.0f%%");
      ImGui::SliderFloat("blur size", &mSOParameters.blurSizePercent, 0, 10, "%.0f%%");

      ImGui::Text("Edge detection parameters");
      ImGui::SliderFloat("edge size", &mSOParameters.soParams.edgeSize, 3, 31, "%.0f");

      ImGui::Text("Objectness parameters");
      ImGui::SliderFloat("objectness radius", &mSOParameters.objectnessRadiusPercent, 1, 10, "%.1f%%");
      ImGui::SliderFloat("objectness threshold", &mSOParameters.soParams.objectnessThreshold, 0, 1, "%.2f");

      ImGui::Text("Object filtering");
      ImGui::Checkbox("draw filtered", &mSOParameters.soParams.drawFiltered);
      ImGui::Checkbox("draw bboxes", &mSOParameters.soParams.drawBboxes);
      ImGui::Checkbox("draw contours", &mSOParameters.soParams.drawContours);
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
  mSOParameters.Update();
}
