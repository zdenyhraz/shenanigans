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
    f32 objectnessRadiusPercent = soParams.objectnessRadius * 100;
    f32 minObjectAreaPercent = soParams.minObjectArea * 100;

    void Update()
    {
      imageSize = imageSizePercent / 100;
      soParams.blurSize = blurSizePercent / 100;
      soParams.objectnessRadius = objectnessRadiusPercent / 100;
      soParams.minObjectArea = minObjectAreaPercent / 100;
    }
  };

  std::string imagePath = "data/ObjectDetection/sonar/tif/sasi-S-upper-20221102-143803-l38.tif";
  std::string imageDirectoryPath = "data/ObjectDetection/sonar/tif";
  std::string saveDirectoryPath = "data/ObjectDetection/sonar/results";
  mutable FilePathGenerator filePathGenerator{GetProjectDirectoryPath(imageDirectoryPath).string()};
  SobelObjectnessWindowParameters mSOParameters;
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
