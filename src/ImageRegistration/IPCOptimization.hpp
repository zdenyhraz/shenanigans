#pragma once

class IPC;

class IPCOptimization
{
public:
  enum OptimizedParameters : u8
  {
    BandpassTypeParameter,
    BandpassLParameter,
    BandpassHParameter,
    InterpolationTypeParameter,
    WindowTypeParameter,
    L2UsizeParameter,
    L1ratioParameter,
    CPepsParameter,
    L1WindowTypeParameter,
    OptimizedParameterCount, // last
  };

  static void Optimize(
      IPC& ipc, const std::string& trainDirectory, const std::string& testDirectory, f64 maxShift = 2.0, f64 noiseStddev = 0.01, i32 iters = 101, f64 testRatio = 0.2, i32 popSize = 42);
  static void Optimize(IPC& ipc, const std::function<f64(const IPC&)>& obj, i32 popSize = 42);

private:
  static std::vector<cv::Mat> LoadImages(const std::string& imagesDirectory, f64 cropSizeRatio = 0.5);
  static std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>> CreateImagePairs(const IPC& ipc, const std::vector<cv::Mat>& images, f64 maxShift, i32 iters, f64 noiseStddev);
  static IPC CreateIPCFromParams(const IPC& ipc, const std::vector<f64>& params);
  static std::function<f64(const std::vector<f64>&)> CreateObjectiveFunction(const IPC& ipc, const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs);
  static std::function<f64(const std::vector<f64>&)> CreateObjectiveFunction(const IPC& ipc, const std::function<f64(const IPC&)>& obj);
  static std::vector<f64> CalculateOptimalParameters(const std::function<f64(const std::vector<f64>&)>& obj, const std::function<f64(const std::vector<f64>&)>& valid, i32 popSize);
  static void ApplyOptimalParameters(IPC& ipc, const std::vector<f64>& optimalParameters);
  static void ShowOptimizationPlots(const std::vector<cv::Point2d>& shiftsReference, const std::vector<cv::Point2d>& shiftsPixel, const std::vector<cv::Point2d>& shiftsNonit,
      const std::vector<cv::Point2d>& shiftsBefore, const std::vector<cv::Point2d>& shiftsAfter);
  static std::vector<cv::Point2d> GetShifts(const IPC& ipc, const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs);
  static std::vector<cv::Point2d> GetNonIterativeShifts(const IPC& ipc, const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs);
  static std::vector<cv::Point2d> GetPixelShifts(const IPC& ipc, const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs);
  static std::vector<cv::Point2d> GetReferenceShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs);
  static f64 GetAverageAccuracy(const std::vector<cv::Point2d>& shiftsReference, const std::vector<cv::Point2d>& shifts);
  static void ShowRandomImagePair(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs);
};
