#pragma once
#include "IPC/IterativePhaseCorrelation.h"

namespace IterativePhaseCorrelationTest
{
bool TestZeroShift()
{
  IterativePhaseCorrelation ipc(100, 100);
  Mat img1 = Mat::ones(1000, 1000, CV_16U);
  Mat img2 = Mat::zeros(1000, 1000, CV_16U);
  Point2f shift = ipc.Calculate(img1, img2);
  return shift == Point2f(0, 0);
}

bool Test() { return TestZeroShift(); }
}
