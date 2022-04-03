#pragma once

struct IPCOptimizeParameters
{
  std::string trainDirectory = "../debug/ipcopt/train";
  std::string testDirectory = "../debug/ipcopt/test";
  f32 maxShift = 2.0;
  f32 noiseStddev = 0.0;
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
  inline static f32 mProgress = 0;
};
