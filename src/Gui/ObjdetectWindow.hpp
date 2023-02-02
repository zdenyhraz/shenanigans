#pragma once
#include "Window.hpp"
#include "ObjectDetection/SobelObjectness.hpp"

class ObjdetectWindow : public Window
{
  struct ObjdetectParameters
  {
    std::string imagePath = "data/debug/ObjectDetection/tif/sasi-S-upper-20221102-144815-l38.tif";
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

  void DetectObjects() const;

  ObjdetectParameters mParameters;

public:
  void Render() override;
};
