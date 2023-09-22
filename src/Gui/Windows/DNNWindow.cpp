#include "DNNWindow.hpp"
#include "DNN/Experiments/ImageSegmentationModel.hpp"
#include "DNN/Experiments/RegressionModel.hpp"
#include "DNN/Experiments/ImageClassificationModel.hpp"
#include "DNN/ObjectDetection/YOLOv8CV.hpp"
#include "DNN/ObjectDetection/YOLOv8Torch.hpp"
#include "DNN/ObjectSegmentation/YOLOv8Torch.hpp"

void DNNWindow::DetectObjectsYOLOv8CVW() const
{
  LOG_FUNCTION;
  const auto image = LoadImage(GetProjectDirectoryPath(imagePath));
  DetectObjectsYOLOv8CV(image, GetProjectDirectoryPath(modelPath + ".onnx"), GetProjectDirectoryPath(classesPath), confidenceThreshold, NMSThreshold);
}

void DNNWindow::DetectObjectsYOLOv8TorchW() const
{
  LOG_FUNCTION;
  const auto image = LoadImage(GetProjectDirectoryPath(imagePath));
  DetectObjectsYOLOv8Torch(image, GetProjectDirectoryPath(modelPath + ".torchscript"), GetProjectDirectoryPath(classesPath), confidenceThreshold, NMSThreshold);
}

void DNNWindow::DetectObjectsYOLOv8SegTorchW() const
{
  LOG_FUNCTION;
  const auto image = LoadImage(GetProjectDirectoryPath(imagePath));
  SegmentObjectsYOLOv8Torch(image, GetProjectDirectoryPath(modelPath + "-seg.torchscript"), GetProjectDirectoryPath(classesPath), confidenceThreshold, NMSThreshold);
}

void DNNWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("DNN"))
  {
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("ML experiments"))
    {
      static i32 modelWidth = 128;
      if (ImGui::Button("Regression model test"))
        LaunchAsync([&]() { RegressionModelTest(modelWidth); });
      ImGui::SliderInt("model width", &modelWidth, 16, 1024);

      if (ImGui::Button("Image segmentation model test"))
        LaunchAsync([&]() { ImageSegmentationModelTest(); });

      if (ImGui::Button("Image classification model test"))
        LaunchAsync([&]() { ImageClassificationModelTest(); });
    }

    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
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
