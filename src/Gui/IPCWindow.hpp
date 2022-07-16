#pragma once
#include "ImageRegistration/IPC.hpp"

struct IPCParameters
{
  i32 Rows = 1024;
  i32 Cols = 1024;
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

class IPCWindow
{
public:
  static void Initialize();
  static void Render();
  static IPC& GetIPC() { return mIPC; }
  static IPC& GetIPCOptimized() { return mIPCOptimized; }

private:
  inline static IPCParameters mParameters;
  inline static IPC mIPC;
  inline static IPC mIPCOptimized;

  static void UpdateIPCParameters(IPC& ipc);
};
