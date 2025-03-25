#pragma once

class IPC;

class IPCFlow
{
public:
  static std::tuple<cv::Mat, cv::Mat> CalculateFlow(const IPC& ipc, const cv::Mat& image1, const cv::Mat& image2, double resolution);
  static std::tuple<cv::Mat, cv::Mat> CalculateFlow(const IPC& ipc, cv::Mat&& image1, cv::Mat&& image2, double resolution);
};
