#pragma once
#include "ImageRegistration/IPC.hpp"

struct IPCParameters
{
  i32 mRows = 64;
  i32 mCols = 64;
  f32 mBPL = 0;
  f32 mBPH = 1;
  i32 mL2size = 7;
  f32 mL1ratio = 0.45;
  i32 mL2Usize = 357;
  i32 mMaxIter = 10;
  f32 mCPeps = 0;
  i32 mWinT = static_cast<i32>(IPC::WindowType::Hann);
  i32 mBPT = static_cast<i32>(IPC::BandpassType::Gaussian);
  i32 mIntT = static_cast<i32>(IPC::InterpolationType::Linear);
  i32 mL1WinT = static_cast<i32>(IPC::L1WindowType::Circular);
  cv::Mat mBP;
  cv::Mat mWin;
  cv::Mat mL1Win;
  static constexpr const char* mWindowTypes[] = {"None", "Hann"};
  static constexpr const char* mBandpassTypes[] = {"None", "Rectangular", "Gaussian"};
  static constexpr const char* mInterpolationTypes[] = {"NearestNeighbor", "Linear", "Cubic"};
  static constexpr const char* mL1windowTypes[] = {"None", "Circular", "Gaussian"};
};

class Shenanigans
{
public:
  static void Run();

private:
  inline static IPCParameters mIPCPars;
  inline static IPC mIPC;

  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void UpdateIPCParameters();
};
