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
  IPC::BandpassType mBPT = IPC::BandpassType::Gaussian;
  IPC::InterpolationType mIntT = IPC::InterpolationType::Linear;
  IPC::WindowType mWinT = IPC::WindowType::Hann;
  IPC::L1WindowType mL1WinT = IPC::L1WindowType::Gaussian;
  cv::Mat mBP;
  cv::Mat mWin;
  cv::Mat mL1Win;
};

class Shenanigans
{
public:
  static void Run();

private:
  inline static IPCParameters mIPCParameters;
  inline static IPC mIPC;

  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void UpdateIPCParameters();
};
