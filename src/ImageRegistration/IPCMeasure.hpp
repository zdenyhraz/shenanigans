#pragma once

class IPC;

class IPCMeasure
{
public:
  static void MeasureAccuracyMap(const IPC& ipc, const cv::Mat& image, i32 iters, f64 maxShift, f64 noiseStddev, f32* progress = nullptr);
};
