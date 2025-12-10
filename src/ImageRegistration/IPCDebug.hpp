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
  static void DebugL1B(const IPC& ipc, const cv::Mat& L2U, int L1size, const cv::Point2d& L3shift, double UC);
  static void DebugL1A(const IPC& ipc, const cv::Mat& L1, const cv::Point2d& L3shift, const cv::Point2d& L2Ushift, double UC, bool last = false);
  static void DebugShift(const IPC& ipc, double maxShift = 2.0, double noiseStdev = 0.01);
  static void DebugShift2(const IPC& ipc, const std::string& image1Path, const std::string& image2Path, double noiseStdev = 0.01);
  static void DebugAlign(const IPC& ipc, const std::string& image1Path, const std::string& image2Path, double noiseStdev = 0.01);
  static void DebugGradualShift(const IPC& ipc, double maxShift = 2.0, double noiseStdev = 0.01);
  static void DebugUC(const IPC& ipc, double maxShift = 2.0, double noiseStdev = 0.01);
};
