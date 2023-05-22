#pragma once
#include "ImageRegistration/IPC.hpp"
#include "Window.hpp"

class IPCWindow : public Window
{
  struct IPCParameters
  {
    i32 Rows = 512;
    i32 Cols = 512;
    f32 BPL = 0;
    f32 BPH = 1;
    i32 L2size = 7;
    f32 L1ratio = 0.45;
    i32 L2Usize = 357;
    i32 MaxIter = 10;
    f32 CPeps = 0;
    i32 WinT = static_cast<i32>(IPC::WindowType::Hann);
    i32 BPT = static_cast<i32>(IPC::BandpassType::Gaussian);
    i32 IntT = static_cast<i32>(IPC::InterpolationType::Linear);
    i32 L1WinT = static_cast<i32>(IPC::L1WindowType::Circular);
    static constexpr const char* WindowTypes[] = {"None", "Hann"};
    static constexpr const char* BandpassTypes[] = {"None", "Rectangular", "Gaussian"};
    static constexpr const char* InterpolationTypes[] = {"NearestNeighbor", "Linear", "Cubic"};
    static constexpr const char* L1WindowTypes[] = {"None", "Circular", "Gaussian"};
  };

  struct IPCOptimizeParameters
  {
    std::string imageDirectory = "../debug/ipcopt/train";
    std::string generateDirectory = "../debug/ipcopt";
    std::string debugImage1Path = "data/debug/1.png";
    std::string debugImage2Path = "data/debug/2.png";
    f32 maxShift = 2.5;
    f32 noiseStddev = 0.0;
    i32 iters = 51;
    f32 testRatio = 0.2;
    i32 popSize = 18;
  };

  void UpdateIPCParameters(IPC& ipc);
  std::string GetCurrentDatasetPath() const;
  void FalseCorrelationsRemoval() const;

  IPCParameters mIPCParameters;
  IPCOptimizeParameters mOptimizeParameters;

public:
  void Initialize() override;
  void Render() override;
};
