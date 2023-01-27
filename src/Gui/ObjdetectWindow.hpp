#pragma once
#include "Window.hpp"
#include "ObjectDetection/SobelObjectness.hpp"

class ObjdetectWindow : public Window
{
  struct ObjdetectParameters
  {
    std::string imagePath = "data/debug/ObjectDetection/input/3.jpg";
    f32 imageSizeMultiplier = 0.25;
    SobelObjectnessParameters soParams;
  };

  void DetectObjects();

  ObjdetectParameters mParameters;

public:
  void Render() override;
};
