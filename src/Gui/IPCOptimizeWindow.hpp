#pragma once

struct IPCOptimizeParameters
{
  std::string debugImage1Path = "../debug/shapes/shape1.png";
  std::string debugImage2Path = "../debug/shapes/shapesf.png";
  std::string trainDirectory = "../debug/ipcopt/train";
  std::string testDirectory = "../debug/ipcopt/test";
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
  inline static f32 mProgress = 0;
};
