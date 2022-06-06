#pragma once

class IPC;

struct ImagePair
{
  cv::Mat image1;
  cv::Mat image2;
  cv::Point2d shift;
  i32 row;
  i32 col;
};

struct ImageRegistrationDataset
{
  std::vector<ImagePair> imagePairs;
  i32 rows;
  i32 cols;
  i32 imageCount;
  i32 iters;
  f64 maxShift;
  f64 noiseStddev;
};

class IPCMeasure
{
  static constexpr f64 mQuanT = 0.95;

public:
  static std::vector<ImagePair> CreateImagePairs(const IPC& ipc, const std::vector<cv::Mat>& images, cv::Point2d maxShift, cv::Point2d shiftOffset1,
      cv::Point2d shiftOffset2, i32 iters, f64 noiseStddev, f32* progress = nullptr);
  static ImageRegistrationDataset LoadImageRegistrationDataset(const std::string& path);
  static void GenerateRegistrationDataset(
      const IPC& ipc, const std::string& path, const std::string& saveDir, i32 iters, f64 maxShiftAbs, f64 noiseStddev, f32* progress = nullptr);
  static void MeasureAccuracy(
      const IPC& ipc, const IPC& ipcopt, const std::string& path, i32 iters, f64 maxShiftAbs, f64 noiseStddev, f32* progress = nullptr);
  static void MeasureAccuracy(const IPC& ipc, const IPC& ipcopt, const std::string& path, f32* progress = nullptr);
};
