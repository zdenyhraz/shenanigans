#pragma once

class IPC;

class IPCMeasure
{
  static constexpr f64 mQuanT = 0.95;

public:
  static void MeasureAccuracyMap(const IPC& ipc, const cv::Mat& image, i32 iters, f64 maxShift, f64 noiseStddev, f32* progress = nullptr);
  static void MeasureAccuracy(const IPC& ipc, const std::string& dataPath, i32 iters, f64 maxShift, f64 noiseStddev, f32* progress = nullptr);

private:
  static std::vector<f64> GetColsMeans(const cv::Mat& mat);
  static std::vector<f64> GetColsStddevs(const cv::Mat& mat);
};
