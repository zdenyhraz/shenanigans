#include "IPCMeasure.hpp"
#include "IPC.hpp"
#include "CrossCorrelation.hpp"
#include "PhaseCorrelation.hpp"
#include "Filtering/Noise.hpp"

void IPCMeasure::MeasureAccuracy(const IPC& ipc, const std::string& path, i32 iters, f64 maxShift, f64 noiseStddev, f32* progress)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("IPCMeasure::MeasureAccuracy");
  LOG_DEBUG("Measuring image {}% registration accuracy on images from {}", mQuanT * 100, path);

  cv::Mat refShiftsX(iters, iters, GetMatType<f64>());
  cv::Mat refShiftsY(iters, iters, GetMatType<f64>());
  cv::Mat accuracyCC(iters, iters, GetMatType<f64>());
  cv::Mat accuracyPC(iters, iters, GetMatType<f64>());
  cv::Mat accuracyPCS(iters, iters, GetMatType<f64>());
  cv::Mat accuracyIPC(iters, iters, GetMatType<f64>());
  cv::Mat accuracyIPCx(iters, iters, GetMatType<f64>());
  cv::Mat accuracyIPCy(iters, iters, GetMatType<f64>());

  std::atomic<i32> progressi = 0;
  i32 imageIndex = 0;
  auto images = LoadImages<f64>(path);
  for (auto& image : images)
    cv::resize(image, image, cv::Size(ipc.mCols + 2 * maxShift + 1, ipc.mRows + 2 * maxShift + 1));

  for (const auto& image : images)
  {
    LOG_FUNCTION(fmt::format("Image {} registration accuracy measurement", imageIndex++));
    cv::Mat image1 = RoiCropMid(image, ipc.mCols, ipc.mRows);
    AddNoise<f64>(image1, noiseStddev);
    PyPlot::Plot("Image", {.z = image1, .cmap = "gray"});

#pragma omp parallel for
    for (i32 row = 0; row < iters; ++row)
    {
      const f64 logprogress = ++progressi;
      if (progress)
        *progress = logprogress / (iters * images.size());
      LOG_DEBUG("[{:>3.0f}% :: {} / {}] Calculating IPC accuracy map ...", logprogress / (iters * images.size()) * 100, logprogress, iters * images.size());

      auto refX = refShiftsX.ptr<f64>(row);
      auto refY = refShiftsY.ptr<f64>(row);
      auto accCC = accuracyCC.ptr<f64>(row);
      auto accPC = accuracyPC.ptr<f64>(row);
      auto accPCS = accuracyPCS.ptr<f64>(row);
      auto accIPC = accuracyIPC.ptr<f64>(row);
      auto accIPCx = accuracyIPCx.ptr<f64>(row);
      auto accIPCy = accuracyIPCy.ptr<f64>(row);

      for (i32 col = 0; col < iters; ++col)
      {
        const auto shift = cv::Point2d(maxShift * (-1.0 + 2.0 * col / (iters - 1)), maxShift * (-1.0 + 2.0 * (iters - 1 - row) / (iters - 1)));
        refX[col] = shift.x;
        refY[col] = shift.y;
        cv::Mat image2 = RoiCropMid(Shifted(image, shift), ipc.mCols, ipc.mRows);
        AddNoise<f64>(image2, noiseStddev);

        accCC[col] += Magnitude(CrossCorrelation::Calculate(image1, image2) - shift);
        accPC[col] += Magnitude(PhaseCorrelation::Calculate(image1, image2) - shift);
        accPCS[col] += Magnitude(cv::phaseCorrelate(image1, image2) - shift);

        const auto errorIPC = ipc.Calculate(image1, image2) - shift;
        accIPC[col] += Magnitude(errorIPC);
        accIPCx[col] += errorIPC.x;
        accIPCy[col] += errorIPC.y;
      }
    }
  }

  accuracyCC = QuantileFilter<f64>(accuracyCC / images.size(), 0, mQuanT);
  accuracyPC = QuantileFilter<f64>(accuracyPC / images.size(), 0, mQuanT);
  accuracyPCS = QuantileFilter<f64>(accuracyPCS / images.size(), 0, mQuanT);
  accuracyIPC = QuantileFilter<f64>(accuracyIPC / images.size(), 0, mQuanT);
  accuracyIPCx = QuantileFilter<f64>(accuracyIPCx / images.size(), 1. - mQuanT, mQuanT);
  accuracyIPCy = QuantileFilter<f64>(accuracyIPCy / images.size(), 1. - mQuanT, mQuanT);

  LOG_SUCCESS("CC average accuracy: {:.3f}", Mean<f64>(accuracyCC), Stddev<f64>(accuracyCC));
  LOG_SUCCESS("PC average accuracy: {:.3f}", Mean<f64>(accuracyPC), Stddev<f64>(accuracyPC));
  LOG_SUCCESS("PCS average accuracy: {:.3f}", Mean<f64>(accuracyPCS), Stddev<f64>(accuracyPCS));
  LOG_SUCCESS("IPC average accuracy: {:.3f}", Mean<f64>(accuracyIPC), Stddev<f64>(accuracyIPC));

  PyPlot::Plot("shift error", "imreg_accuracy",
      py::dict{"x"_a = ColMeans<f64>(refShiftsX), "pc_error"_a = ColMeans<f64>(accuracyPC), "pc_stddev"_a = ColStddevs<f64>(accuracyPC), "pcs_error"_a = ColMeans<f64>(accuracyPCS),
          "pcs_stddev"_a = ColStddevs<f64>(accuracyPCS), "ipc_error"_a = ColMeans<f64>(accuracyIPC), "ipc_stddev"_a = ColStddevs<f64>(accuracyIPC)});

  static constexpr auto xlabel = "x shift";
  static constexpr auto ylabel = "y shift";
  static constexpr auto zlabel = "error";
  const auto [xmin, xmax] = MinMax(refShiftsX);
  const auto [ymin, ymax] = MinMax(refShiftsX);
  PyPlot::Plot("CC accuracy map", {.z = accuracyCC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("PC accuracy map", {.z = accuracyPC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("PCS accuracy map", {.z = accuracyPCS, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("IPC accuracy map", {.z = accuracyIPC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("IPCx accuracy map", {.z = accuracyIPCx, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("IPCy accuracy map", {.z = accuracyIPCy, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});

  if (progress)
    *progress = 0;
}
