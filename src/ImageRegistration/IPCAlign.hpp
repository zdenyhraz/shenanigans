#pragma once

class IPC;

class IPCAlign
{
public:
  static cv::Mat Align(const IPC& ipc, const cv::Mat& image1, const cv::Mat& image2);
  static cv::Mat Align(const IPC& ipc, cv::Mat&& image1, cv::Mat&& image2);

private:
  static cv::Mat ColorComposition(const cv::Mat& img1, const cv::Mat& img2, f64 gamma1 = 1, f64 gamma2 = 1);
};
