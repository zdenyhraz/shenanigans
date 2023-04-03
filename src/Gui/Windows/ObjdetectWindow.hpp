#pragma once
#include "Gui/Windows/Window.hpp"
#include "SobelObjectness.hpp"

class ObjdetectWindow : public Window
{
  std::string imagePath = "data/ObjectDetection/sonar/tif/sasi-S-upper-20221102-143803-l38.tif";
  std::string imageDirectoryPath = "data/ObjectDetection/sonar/tif";
  mutable FilePathGenerator filePathGenerator{GetProjectDirectoryPath(imageDirectoryPath).string()};
  SobelObjectnessParameters mSOParameters;
  std::string modelPath = "data/DNN/yolov8/yolov8x";
  std::string classesPath = "data/DNN/yolov8/coco.names";
  f32 confidenceThreshold = 0.05;
  f32 NMSThreshold = 0.5;

  void DetectObjectsSO() const;
  SobelObjectnessParameters OptimizeSOParameters() const;
  void DetectObjectsSODirectory() const;
  void DetectObjectsYOLOv8CVW() const;
  void DetectObjectsYOLOv8TorchW() const;
  void DetectObjectsYOLOv8SegTorchW() const;

public:
  void Render() override;
};
