#pragma once

class IPC;

struct ImagePair
{
  cv::Mat image1;
  cv::Mat image2;
  cv::Point2d shift;
  int row;
  int col;
};

struct ImageRegistrationDataset
{
  std::vector<ImagePair> imagePairs;
  int rows;
  int cols;
  int imageCount;
  int iters;
  double maxShift;
  double noiseStddev;
};

std::vector<ImagePair> CreateImagePairs(const IPC& ipc, const std::vector<cv::Mat>& images, cv::Point2d maxShift, cv::Point2d shiftOffset1, cv::Point2d shiftOffset2, int iters,
    double noiseStddev, float* progress = nullptr);
ImageRegistrationDataset LoadImageRegistrationDataset(const std::string& path);
void GenerateImageRegistrationDataset(
    const IPC& ipc, const std::string& path, const std::string& saveDir, int iters, double maxShiftAbs, double noiseStddev, float* progress = nullptr);
