#include "IPCMeasure.hpp"
#include "IPC.hpp"
#include "CrossCorrelation.hpp"
#include "PhaseCorrelation.hpp"

void IPCMeasure::MeasureAccuracyMap(const IPC& ipc, const cv::Mat image, i32 n)
{
  cv::Mat accuracyMapCC(n, n, GetMatType<IPC::Float>());
  cv::Mat accuracyMapPC(n, n, GetMatType<IPC::Float>());
  cv::Mat accuracyMapPCS(n, n, GetMatType<IPC::Float>());
  cv::Mat accuracyMapIPC(n, n, GetMatType<IPC::Float>());

  const f64 maxShift = 2.0;
  const std::string xlabel = "x shift";
  const std::string ylabel = "y shift";
  const std::string zlabel = "error";
  const cv::Point2i point(std::clamp(Random::Rand() * image.cols, static_cast<f64>(ipc.mCols + 2. * maxShift), static_cast<f64>(image.cols - ipc.mCols - 2. * maxShift)),
      std::clamp(Random::Rand() * image.rows, static_cast<f64>(ipc.mRows + 2. * maxShift), static_cast<f64>(image.rows - ipc.mRows - 2. * maxShift)));
  const cv::Mat image1 = RoiCrop(image, point.x, point.y, ipc.mCols, ipc.mRows);
  std::atomic<i32> progress = 0;

#pragma omp parallel for
  for (i32 row = 0; row < n; ++row)
  {
    const f64 logprogress = progress++ + 1;
    LOG_INFO("[{:>3.0f}% :: {} / {}] Calculating IPC accuracy map ...", logprogress / n * 100, logprogress, n);

    auto accCC = accuracyMapCC.ptr<IPC::Float>(row);
    auto accPC = accuracyMapPC.ptr<IPC::Float>(row);
    auto accPCS = accuracyMapPCS.ptr<IPC::Float>(row);
    auto accIPC = accuracyMapIPC.ptr<IPC::Float>(row);
    for (i32 col = 0; col < n; ++col)
    {
      const auto shift = cv::Point2d(maxShift * (-1.0 + 2.0 * col / (n - 1)), maxShift * (-1.0 + 2.0 * (n - 1 - row) / (n - 1)));
      const cv::Mat Tmat = (cv::Mat_<f64>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
      cv::Mat imageShifted;
      cv::warpAffine(image, imageShifted, Tmat, image.size());
      cv::Mat image2 = RoiCrop(imageShifted, point.x, point.y, ipc.mCols, ipc.mRows);

      accCC[col] = Magnitude(CrossCorrelation::Calculate(image1, image2) - shift);
      accPC[col] = Magnitude(PhaseCorrelation::Calculate(image1, image2) - shift);
      accPCS[col] = Magnitude(ipc.Calculate<{.AccuracyT = IPC::AccuracyType::Subpixel}>(image1, image2) - shift);
      accIPC[col] = Magnitude(ipc.Calculate(image1, image2) - shift);
    }
  }

  PyPlot::Plot("Image", {.z = image1, .cmap = "gray"});
  PyPlot::Plot("CC accuracy map", {.z = accuracyMapCC, .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("PC accuracy map", {.z = accuracyMapPC, .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("PCS accuracy map", {.z = accuracyMapPCS, .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("IPC accuracy map", {.z = accuracyMapIPC, .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
}
