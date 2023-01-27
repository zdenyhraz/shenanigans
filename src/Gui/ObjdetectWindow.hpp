#pragma once
#include "Gui.hpp"
#include "Random/ObjectDetection.hpp"

struct ObjdetectParameters
{
  std::string imagePath = "data/debug/ObjectDetection/input/3.jpg";
  f32 imageSizeMultiplier = 0.25;
  SobelObjectnessParameters soParams;
};

class ObjdetectWindow
{
  static void DetectObjects();

  inline static ObjdetectParameters mParameters;

public:
  static void Render();
};
