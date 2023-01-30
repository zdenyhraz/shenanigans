#pragma once
#include "Window.hpp"
#include "ObjectDetection/SobelObjectness.hpp"

class ObjdetectWindow : public Window
{
  struct ObjdetectParameters
  {
    std::string imagePath = "data/debug/ObjectDetection/input/main.tif";
    f32 imageSizeMultiplier = 0.25;
    SobelObjectnessParameters soParams;
  };

  void DetectObjects() const;

  ObjdetectParameters mParameters;

public:
  void Render() override;
};
