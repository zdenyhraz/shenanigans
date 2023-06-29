#pragma once
#include "Gui/Windows/Window.hpp"

class DNNWindow : public Window
{
  std::string imagePath = "data/ObjectDetection/sonar/interesting/sasi-S-upper-20221102-143803-l38.tif";
  std::string modelPath = "data/DNN/yolov8/yolov8x";
  std::string classesPath = "data/DNN/yolov8/coco.names";
  f32 confidenceThreshold = 0.05;
  f32 NMSThreshold = 0.5;

  void DetectObjectsYOLOv8CVW() const;
  void DetectObjectsYOLOv8TorchW() const;
  void DetectObjectsYOLOv8SegTorchW() const;

public:
  void Render() override;
};
