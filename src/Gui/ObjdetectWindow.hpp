#pragma once
#include "Window.hpp"
#include "ObjectDetection/SobelObjectness.hpp"

class ObjdetectWindow : public Window
{
  struct ObjdetectParameters
  {
    std::string imagePath = "data/debug/ObjectDetection/tif/sasi-S-upper-20221102-144815-l38.tif";
    f32 imageSizeMultiplier = 0.25;
    SobelObjectnessParameters soParams;
  };

  void DetectObjects() const;

  ObjdetectParameters mParameters;

public:
  void Render() override;
};
