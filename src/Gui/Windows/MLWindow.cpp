#include "MLWindow.hpp"
#include "ML/Experiments/ImageSegmentationModel.hpp"
#include "ML/Experiments/RegressionModel.hpp"
#include "ML/Experiments/ImageClassificationModel.hpp"
#include "ML/ObjectDetection/YOLOv8CV.hpp"
#include "ML/ObjectDetection/YOLOv8Torch.hpp"
#include "ML/ObjectSegmentation/YOLOv8Torch.hpp"
#include "NDA/FaceDetector.hpp"

void MLWindow::DetectObjectsYOLOv8CVW() const
{
  LOG_FUNCTION;
  const auto image = LoadImage(GetProjectDirectoryPath(imagePath));
  DetectObjectsYOLOv8CV(image, GetProjectDirectoryPath(modelPath + ".onnx"), GetProjectDirectoryPath(classesPath), confidenceThreshold, NMSThreshold);
}

void MLWindow::DetectObjectsYOLOv8TorchW() const
{
  LOG_FUNCTION;
  const auto image = LoadImage(GetProjectDirectoryPath(imagePath));
  DetectObjectsYOLOv8Torch(image, GetProjectDirectoryPath(modelPath + ".torchscript"), GetProjectDirectoryPath(classesPath), confidenceThreshold, NMSThreshold);
}

void MLWindow::DetectObjectsYOLOv8SegTorchW() const
{
  LOG_FUNCTION;
  const auto image = LoadImage(GetProjectDirectoryPath(imagePath));
  SegmentObjectsYOLOv8Torch(image, GetProjectDirectoryPath(modelPath + "-seg.torchscript"), GetProjectDirectoryPath(classesPath), confidenceThreshold, NMSThreshold);
}

void MLWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("ML"))
  {
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("ML experiments"))
    {
      static i32 modelWidth = 128;
      if (ImGui::Button("Regression model test"))
        LaunchAsync([&]() { RegressionModelTest(modelWidth); });
      ImGui::SliderInt("model width", &modelWidth, 16, 1024);

      if (ImGui::Button("Image segmentation model"))
        LaunchAsync([&]() { ImageSegmentationModelTest(); });

      if (ImGui::Button("Image classification model"))
        LaunchAsync([&]() { ImageClassificationModelTest(); });

      if (ImGui::Button("Face detector OpenCV"))
        LaunchAsync([&]() { FaceDetectorTest(); });
    }

    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Object detection via ML"))
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
