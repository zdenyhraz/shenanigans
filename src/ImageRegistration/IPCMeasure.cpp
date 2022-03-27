#include "IPCMeasure.hpp"
#include "IPC.hpp"

void IPCMeasure::MeasureAccuracyMap(const IPC& ipc, const cv::Mat image, i32 n)
{
  cv::Mat accuracyMap(n, n, GetMatType<IPC::Float>());
  const f64 maxShift = 3.0;
  const cv::Point2i point(std::clamp(Random::Rand() * image.cols, static_cast<f64>(ipc.mCols + 2. * maxShift), static_cast<f64>(image.cols - ipc.mCols - 2. * maxShift)),
      std::clamp(Random::Rand() * image.rows, static_cast<f64>(ipc.mRows + 2. * maxShift), static_cast<f64>(image.rows - ipc.mRows - 2. * maxShift)));
  const cv::Mat image1 = RoiCrop(image, point.x, point.y, ipc.mCols, ipc.mRows);
  std::atomic<i32> progress = 0;

#pragma omp parallel for
  for (i32 row = 0; row < n; ++row)
  {
    const f64 logprogress = progress++ + 1;
    LOG_DEBUG("[{:>3.0f}% :: {} / {}] Calculating IPC accuracy map ...", logprogress / n * 100, logprogress, n);
    auto acc = accuracyMap.ptr<IPC::Float>(row);
    for (i32 col = 0; col < n; ++col)
    {
      const auto shift = cv::Point2d(maxShift * (-1.0 + 2.0 * col / (n - 1)), maxShift * (-1.0 + 2.0 * (n - 1 - row) / (n - 1)));
      const cv::Mat Tmat = (cv::Mat_<f64>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
      cv::Mat imageShifted;
      cv::warpAffine(image, imageShifted, Tmat, image.size());
      cv::Mat image2 = RoiCrop(imageShifted, point.x, point.y, ipc.mCols, ipc.mRows);

      const auto shiftEst = ipc.Calculate<{.AccuracyT = IPC::AccuracyType::Subpixel}>(image1.clone(), std::move(image2));
      const auto error = shiftEst - shift;
      acc[col] = std::sqrt(std::pow(error.x, 2) + std::pow(error.y, 2));
    }
  }

  PyPlot::Plot("IPC accuracy map", {.z = accuracyMap, .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift});
}
