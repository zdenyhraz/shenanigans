#pragma once

struct IPCOptimizeParameters
{
  std::string imagePath = "../debug/ipcopt/shape.png";
  std::string trainDirectory = "../debug/ipcopt";
  std::string testDirectory = "../debug/ipcopt";
  f32 maxShift = 2.0;
  f32 noiseStddev = 0.01;
  i32 iters = 101;
  i32 optiters = 11;
  f32 testRatio = 0.2;
  i32 popSize = 42;
};

class IPCOptimizeWindow
{
public:
  static void Initialize();
  static void Render();

private:
  inline static IPCOptimizeParameters mParameters;
  inline static cv::Mat mImage;
  inline static f32 mProgress = 0;
};
