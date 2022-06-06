#pragma once
#include "Gui.hpp"

struct IPCOptimizeParameters
{
  std::string measureDirectory = "../debug/ipcopt/imreg_dataset_64x64_31i_0ns";
  std::string trainDirectory = "../debug/ipcopt/train";
  std::string generateDirectory = "../debug/ipcopt";
  std::string debugImage1Path = "../debug/shapes/shape1.png";
  std::string debugImage2Path = "../debug/shapes/shapesf.png";
  f32 maxShift = 2.5;
  f32 noiseStddev = 0.0;
  i32 iters = 101;
  i32 optiters = 101;
  f32 testRatio = 0.2;
  i32 popSize = 12;
};

class IPCOptimizeWindow
{
public:
  static void Initialize();
  static void Render();

private:
  inline static IPCOptimizeParameters mParameters;
  inline static ProgressStatus mProgressStatus;
};
