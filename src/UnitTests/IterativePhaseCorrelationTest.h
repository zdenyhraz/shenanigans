#pragma once
#include "IPC/IterativePhaseCorrelation.h"

namespace IterativePhaseCorrelationTest
{
bool TestZeroShift()
{
  IterativePhaseCorrelation ipc(100, 100);
  cv::Mat img1 = cv::Mat::ones(1000, 1000, CV_16U);
  cv::Mat img2 = cv::Mat::zeros(1000, 1000, CV_16U);
  cv::Point2f shift = ipc.Calculate(img1, img2);
  return shift == cv::Point2f(0, 0);
}

bool Test() { return TestZeroShift(); }
}
