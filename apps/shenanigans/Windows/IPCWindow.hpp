#pragma once
#include "ImageRegistration/IPC.hpp"
#include "Gui/Window.hpp"

class IPCWindow : public Window
{
  struct IPCParameters
  {
    int Rows = 512;
    int Cols = 512;
    float BPL = 0;
    float BPH = 1;
    int L2size = 7;
    float L1ratio = 0.45;
    int L2Usize = 357;
    int MaxIter = 10;
    float CPeps = 0;
    int WinT = static_cast<int>(IPC::WindowType::Hann);
    int BPT = static_cast<int>(IPC::BandpassType::Gaussian);
    int IntT = static_cast<int>(IPC::InterpolationType::Linear);
    int L1WinT = static_cast<int>(IPC::L1WindowType::Circular);
    static constexpr const char* WindowTypes[] = {"None", "Hann"};
    static constexpr const char* BandpassTypes[] = {"None", "Rectangular", "Gaussian"};
    static constexpr const char* InterpolationTypes[] = {"NearestNeighbor", "Linear", "Cubic"};
    static constexpr const char* L1WindowTypes[] = {"None", "Circular", "Gaussian"};
  };

  struct IPCOptimizeParameters
  {
    std::string imageDirectory = "data/ipc/images";
    std::string generateDirectory = "data/ipc/pairs";
    std::string debugImage1Path = "data/debug/artificial1.png";
    std::string debugImage2Path = "data/debug/artificial2.png";
    float maxShift = 4;
    float noiseStddev = 0.0;
    int iters = 101;
    float testRatio = 0.2;
    int popSize = 18;
  };

  void UpdateIPCParameters(IPC& ipc);
  std::string GetCurrentDatasetPath() const;
  void FalseCorrelationsRemoval();

  IPCParameters mIPCParameters;
  IPCOptimizeParameters mOptimizeParameters;
  IPC mIPC;
  IPC mIPCOptimized;

public:
  void Initialize() override;
  void Render() override;
};
