#pragma once
#include "Window.hpp"
#include "Random/ObjectDetection.hpp"

struct ObjdetectParameters
{
  std::string imagePath = "data/debug/ObjectDetection/input/3.jpg";
  f32 imageSizeMultiplier = 0.25;
  SobelObjectnessParameters soParams;
};

class ObjdetectWindow : public Window
{
  void DetectObjects();

  ObjdetectParameters mParameters;

public:
  void Render() override;
};
