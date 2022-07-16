#pragma once
#include "Gui.hpp"

struct IPCOptimizeParameters
{
  std::string imageDirectory = "../debug/ipcopt/train";
  std::string generateDirectory = "../debug/ipcopt";
  std::string debugImage1Path = "../debug/shapes/shape1.png"; //"../debug/AIA/304A.png";
  std::string debugImage2Path = "../debug/shapes/shape2.png"; //"../debug/AIA/171A.png";
  f32 maxShift = 2.5;
  f32 noiseStddev = 0.0;
  i32 iters = 51;
  f32 testRatio = 0.2;
  i32 popSize = 18;
};

class IPCOptimizeWindow
{
public:
  static void Initialize();
  static void Render();

private:
  inline static IPCOptimizeParameters mParameters;
  inline static ProgressStatus mProgressStatus;

  static std::string GetCurrentDatasetPath();
};
