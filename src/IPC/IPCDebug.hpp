#pragma once

class IPC;

class IPCDebug
{
public:
  static void DebugInputImages(const IPC& ipc, const cv::Mat& image1, const cv::Mat& image2);
  static void DebugFourierTransforms(const IPC& ipc, const cv::Mat& dft1, const cv::Mat& dft2);
  static void DebugCrossPowerSpectrum(const IPC& ipc, const cv::Mat& crosspower);
  static void DebugL3(const IPC& ipc, const cv::Mat& L3);
  static void DebugL2(const IPC& ipc, const cv::Mat& L2);
  static void DebugL2U(const IPC& ipc, const cv::Mat& L2, const cv::Mat& L2U);
  static void DebugL1B(const IPC& ipc, const cv::Mat& L2U, i32 L1size, const cv::Point2d& L3shift, f64 UC);
  static void DebugL1A(const IPC& ipc, const cv::Mat& L1, const cv::Point2d& L3shift, const cv::Point2d& L2Ushift, f64 UC, bool last = false);
  static void ShowDebugStuff(const IPC& ipc);
};
