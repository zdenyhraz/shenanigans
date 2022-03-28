#include "IPCMeasure.hpp"
#include "IPC.hpp"
#include "CrossCorrelation.hpp"
#include "PhaseCorrelation.hpp"
#include "Filtering/Noise.hpp"

void IPCMeasure::MeasureAccuracyMap(const IPC& ipc, const cv::Mat image, i32 n)
{
  using T = IPC::Float;
  cv::Mat accuracyMapCC(n, n, GetMatType<T>());
  cv::Mat accuracyMapPC(n, n, GetMatType<T>());
  cv::Mat accuracyMapPCS(n, n, GetMatType<T>());
  cv::Mat accuracyMapIPC(n, n, GetMatType<T>());
  cv::Mat accuracyMapIPCx(n, n, GetMatType<T>());
  cv::Mat accuracyMapIPCy(n, n, GetMatType<T>());

  static constexpr f64 maxShift = 2.0;
  static constexpr f64 noiseStddev = 0.1;
  static constexpr f64 quanB = 0.0;
  static constexpr f64 quanT = 0.95;
  const std::string xlabel = "x shift";
  const std::string ylabel = "y shift";
  const std::string zlabel = "error";
  cv::Mat imageres;
  cv::resize(image, imageres, cv::Size(ipc.mCols + 3 * maxShift, ipc.mRows + 3 * maxShift));

  cv::Mat image1 = RoiCropMid(imageres, ipc.mCols, ipc.mRows);
  std::atomic<i32> progress = 0;
  AddNoise<T>(image1, noiseStddev);
  PyPlot::Plot("Image", {.z = image1, .cmap = "gray"});

#pragma omp parallel for
  for (i32 row = 0; row < n; ++row)
  {
    const f64 logprogress = progress++ + 1;
    LOG_INFO("[{:>3.0f}% :: {} / {}] Calculating IPC accuracy map ...", logprogress / n * 100, logprogress, n);

    auto accCC = accuracyMapCC.ptr<T>(row);
    auto accPC = accuracyMapPC.ptr<T>(row);
    auto accPCS = accuracyMapPCS.ptr<T>(row);
    auto accIPC = accuracyMapIPC.ptr<T>(row);
    auto accIPCx = accuracyMapIPCx.ptr<T>(row);
    auto accIPCy = accuracyMapIPCy.ptr<T>(row);

    for (i32 col = 0; col < n; ++col)
    {
      const auto shift = cv::Point2d(maxShift * (-1.0 + 2.0 * col / (n - 1)), maxShift * (-1.0 + 2.0 * (n - 1 - row) / (n - 1)));
      const cv::Mat Tmat = (cv::Mat_<f64>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
      cv::Mat imageShifted;
      cv::warpAffine(imageres, imageShifted, Tmat, imageres.size());
      cv::Mat image2 = RoiCropMid(imageShifted, ipc.mCols, ipc.mRows);
      AddNoise<T>(image2, noiseStddev);

      accCC[col] = Magnitude(CrossCorrelation::Calculate(image1, image2) - shift);
      accPC[col] = Magnitude(PhaseCorrelation::Calculate(image1, image2) - shift);
      accPCS[col] = Magnitude(cv::phaseCorrelate(image1, image2) - shift);

      const auto errorIPC = ipc.Calculate(image1, image2) - shift;
      accIPC[col] = Magnitude(errorIPC);
      accIPCx[col] = errorIPC.x;
      accIPCy[col] = errorIPC.y;
    }
  }

  PyPlot::Plot("CC accuracy map",
      {.z = ApplyQuantile<T>(accuracyMapCC, quanB, quanT), .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("PC accuracy map",
      {.z = ApplyQuantile<T>(accuracyMapPC, quanB, quanT), .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("PCS accuracy map",
      {.z = ApplyQuantile<T>(accuracyMapPCS, quanB, quanT), .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("IPC accuracy map",
      {.z = ApplyQuantile<T>(accuracyMapIPC, quanB, quanT), .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("IPCx accuracy map",
      {.z = ApplyQuantile<T>(accuracyMapIPCx, 1. - quanT, quanT), .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("IPCy accuracy map",
      {.z = ApplyQuantile<T>(accuracyMapIPCy, 1. - quanT, quanT), .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
}
