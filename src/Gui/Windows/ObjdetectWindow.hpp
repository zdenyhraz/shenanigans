#pragma once
#include "Gui/Windows/Window.hpp"
#include "SobelObjectness.hpp"

class ObjdetectWindow : public Window
{
  struct SobelObjectnessWindowParameters
  {
    f32 imageSize = 0.25;
    SobelObjectnessParameters soParams;

    f32 imageSizePercent = imageSize * 100;
    f32 blurSizePercent = soParams.blurSize * 100;
    f32 edgeSizePercent = soParams.edgeSize * 100;
    f32 edgeThresholdPercent = soParams.edgeThreshold * 100;
    f32 objectSizePercent = soParams.objectSize * 100;
    f32 objectnessThresholdPercent = soParams.objectnessThreshold * 100;
    f32 minObjectAreaPercent = soParams.minObjectArea * 100;

    void Update()
    {
      imageSize = imageSizePercent / 100;
      soParams.blurSize = blurSizePercent / 100;
      soParams.edgeSize = edgeSizePercent / 100;
      soParams.edgeThreshold = edgeThresholdPercent / 100;
      soParams.objectSize = objectSizePercent / 100;
      soParams.objectnessThreshold = objectnessThresholdPercent / 100;
      soParams.minObjectArea = minObjectAreaPercent / 100;
    }
  };

  std::string modelPath = "data/DNN/yolov8/yolov8n";
  std::string imagePath = "data/ObjectDetection/cats2.jpg";
  std::string classesPath = "data/DNN/yolov8/coco.names";
  f32 confidenceThreshold = 0.25;
  f32 NMSThreshold = 0.5;

  void DetectObjectsSO() const;
  void DetectObjectsYOLOv3CVW() const;
  void DetectObjectsYOLOv8CVW() const;
  void DetectObjectsYOLOv8TorchW() const;

  SobelObjectnessWindowParameters mSOParameters;

public:
  void Render() override;
};
