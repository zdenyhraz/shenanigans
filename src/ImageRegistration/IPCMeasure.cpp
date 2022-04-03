#include "IPCMeasure.hpp"
#include "IPC.hpp"
#include "CrossCorrelation.hpp"
#include "PhaseCorrelation.hpp"
#include "Filtering/Noise.hpp"

void IPCMeasure::MeasureAccuracyMap(const IPC& ipc, const cv::Mat& image, i32 iters, f64 maxShift, f64 noiseStddev, f32* progress)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("IPCMeasure::MeasureAccuracyMap");

  cv::Mat refShiftsX(iters, iters, GetMatType<f64>());
  cv::Mat accuracyMapCC(iters, iters, GetMatType<f64>());
  cv::Mat accuracyMapPC(iters, iters, GetMatType<f64>());
  cv::Mat accuracyMapPCS(iters, iters, GetMatType<f64>());
  cv::Mat accuracyMapIPC(iters, iters, GetMatType<f64>());
  cv::Mat accuracyMapIPCx(iters, iters, GetMatType<f64>());
  cv::Mat accuracyMapIPCy(iters, iters, GetMatType<f64>());

  static const std::string xlabel = "x shift";
  static const std::string ylabel = "y shift";
  static const std::string zlabel = "error";
  cv::Mat imageres;
  cv::resize(image, imageres, cv::Size(ipc.mCols + 3 * maxShift, ipc.mRows + 3 * maxShift));

  cv::Mat image1 = RoiCropMid(imageres, ipc.mCols, ipc.mRows);
  std::atomic<i32> progressi = 0;
  AddNoise<f64>(image1, noiseStddev);
  PyPlot::Plot("Image", {.z = image1, .cmap = "gray"});

#pragma omp parallel for
  for (i32 row = 0; row < iters; ++row)
  {
    const f64 logprogress = ++progressi;
    if (progress)
      *progress = logprogress / iters;
    LOG_DEBUG("[{:>3.0f}% :: {} / {}] Calculating IPC accuracy map ...", logprogress / iters * 100, logprogress, iters);

    auto refX = refShiftsX.ptr<f64>(row);
    auto accCC = accuracyMapCC.ptr<f64>(row);
    auto accPC = accuracyMapPC.ptr<f64>(row);
    auto accPCS = accuracyMapPCS.ptr<f64>(row);
    auto accIPC = accuracyMapIPC.ptr<f64>(row);
    auto accIPCx = accuracyMapIPCx.ptr<f64>(row);
    auto accIPCy = accuracyMapIPCy.ptr<f64>(row);

    for (i32 col = 0; col < iters; ++col)
    {
      const auto shift = cv::Point2d(maxShift * (-1.0 + 2.0 * col / (iters - 1)), maxShift * (-1.0 + 2.0 * (iters - 1 - row) / (iters - 1)));
      refX[col] = shift.x;
      const cv::Mat Tmat = (cv::Mat_<f64>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
      cv::Mat imageShifted;
      cv::warpAffine(imageres, imageShifted, Tmat, imageres.size());
      cv::Mat image2 = RoiCropMid(imageShifted, ipc.mCols, ipc.mRows);
      AddNoise<f64>(image2, noiseStddev);

      accCC[col] = Magnitude(CrossCorrelation::Calculate(image1, image2) - shift);
      accPC[col] = Magnitude(PhaseCorrelation::Calculate(image1, image2) - shift);
      accPCS[col] = Magnitude(cv::phaseCorrelate(image1, image2) - shift);

      const auto errorIPC = ipc.Calculate(image1, image2) - shift;
      accIPC[col] = Magnitude(errorIPC);
      accIPCx[col] = errorIPC.x;
      accIPCy[col] = errorIPC.y;
    }
  }

  accuracyMapCC = ApplyQuantile<f64>(accuracyMapCC, 0, mQuanT);
  accuracyMapPC = ApplyQuantile<f64>(accuracyMapPC, 0, mQuanT);
  accuracyMapPCS = ApplyQuantile<f64>(accuracyMapPCS, 0, mQuanT);
  accuracyMapIPC = ApplyQuantile<f64>(accuracyMapIPC, 0, mQuanT);
  accuracyMapIPCx = ApplyQuantile<f64>(accuracyMapIPCx, 1. - mQuanT, mQuanT);
  accuracyMapIPCy = ApplyQuantile<f64>(accuracyMapIPCy, 1. - mQuanT, mQuanT);

  PyPlot::Plot("shift error", "imreg_accuracy",
      py::dict{"x"_a = GetColsMeans(refShiftsX), "pcs_error"_a = GetColsMeans(accuracyMapPCS), "pcs_stddev"_a = GetColsStddevs(accuracyMapPCS), "ipc_error"_a = GetColsMeans(accuracyMapIPC),
          "ipc_stddev"_a = GetColsStddevs(accuracyMapIPC)});

  PyPlot::Plot("CC accuracy map", {.z = accuracyMapCC, .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("PC accuracy map", {.z = accuracyMapPC, .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("PCS accuracy map", {.z = accuracyMapPCS, .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("IPC accuracy map", {.z = accuracyMapIPC, .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("IPCx accuracy map", {.z = accuracyMapIPCx, .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
  PyPlot::Plot("IPCy accuracy map", {.z = accuracyMapIPCy, .xmin = -maxShift, .xmax = maxShift, .ymin = -maxShift, .ymax = maxShift, .xlabel = xlabel, .ylabel = ylabel, .zlabel = zlabel});
}

void IPCMeasure::MeasureAccuracy(const IPC& ipc, const std::string& dataPath, i32 iters, f64 maxShift, f64 noiseStddev, f32* progress)
{
}

std::vector<f64> IPCMeasure::GetColsMeans(const cv::Mat& mat)
{
  std::vector<f64> averages(mat.cols, 0.);

  for (i32 c = 0; c < mat.cols; ++c)
  {
    for (i32 r = 0; r < mat.rows; ++r)
      averages[c] += mat.at<f64>(r, c);

    averages[c] /= mat.rows;
  }
  return averages;
}

std::vector<f64> IPCMeasure::GetColsStddevs(const cv::Mat& mat)
{
  std::vector<f64> stddevs(mat.cols, 0.);
  const auto means = GetColsMeans(mat);

  for (i32 c = 0; c < mat.cols; ++c)
  {
    for (i32 r = 0; r < mat.rows; ++r)
      stddevs[c] += std::pow(mat.at<f64>(r, c) - means[c], 2);

    stddevs[c] /= mat.rows;
    stddevs[c] = std::sqrt(stddevs[c]);
  }
  return stddevs;
}
