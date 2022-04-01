#pragma once

class IPC;

class IPCMeasure
{
public:
  static void MeasureAccuracyMap(const IPC& ipc, const cv::Mat& image, i32 n);
};
