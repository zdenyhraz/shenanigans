#pragma once
#include "Window.hpp"
#include "Gui.hpp"

struct IPCOptimizeParameters
{
  std::string imageDirectory = "../debug/ipcopt/train";
  std::string generateDirectory = "../debug/ipcopt";
  std::string debugImage1Path = "../debug/AIA/304A.png";
  std::string debugImage2Path = "../debug/AIA/171A.png";
  f32 maxShift = 2.5;
  f32 noiseStddev = 0.0;
  i32 iters = 51;
  f32 testRatio = 0.2;
  i32 popSize = 18;
};

class IPCAppsWindow : public Window
{
  std::string GetCurrentDatasetPath();
  void FalseCorrelationsRemoval();

  IPCOptimizeParameters mParameters;
  ProgressStatus mProgressStatus;

public:
  void Render() override;
};
