#pragma once

struct IPCOptimizeParameters
{
  std::string trainDirectory;
  std::string testDirectory;
  f32 maxShift = 2.0;
  f32 noiseStddev = 0.01;
  i32 iters = 11;
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
};
