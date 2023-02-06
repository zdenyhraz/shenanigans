#include "IPCMeasure.hpp"
#include "IPC.hpp"
#include "CrossCorrelation.hpp"
#include "PhaseCorrelation.hpp"
#include "PhaseCorrelationUpscale.hpp"
#include "Math/Quantile.hpp"

void IPCMeasure::MeasureAccuracy(const IPC& ipc, const IPC& ipcopt, const std::string& path)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  const auto dataset = LoadImageRegistrationDataset(path);
  const auto iters = dataset.iters;

  cv::Mat refShiftsX = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat refShiftsY = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat accuracyPC = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat accuracyPCS = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat accuracyIPC = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat accuracyIPCO = cv::Mat::zeros(iters, iters, GetMatType<f64>());

  std::atomic<usize> iprogress = 0;
#pragma omp parallel for
  for (i32 idx = 0; idx < dataset.imagePairs.size(); ++idx)
  {
    const auto& image1 = dataset.imagePairs[idx].image1;
    const auto& image2 = dataset.imagePairs[idx].image2;
    const auto& shift = dataset.imagePairs[idx].shift;
    const auto row = dataset.imagePairs[idx].row;
    const auto col = dataset.imagePairs[idx].col;

    refShiftsX.at<f64>(row, col) = shift.x;
    refShiftsY.at<f64>(row, col) = shift.y;
    accuracyPC.at<f64>(row, col) += Magnitude(PhaseCorrelation::Calculate(image1, image2) - shift);
    accuracyPCS.at<f64>(row, col) += Magnitude(cv::phaseCorrelate(image1, image2) - shift);
    accuracyIPC.at<f64>(row, col) += Magnitude(ipc.Calculate(image1, image2) - shift);
    accuracyIPCO.at<f64>(row, col) += Magnitude(ipcopt.Calculate(image1, image2) - shift);

    if (idx == 0)
    {
      if (true)
      {
        PyPlot::Plot({.name = "Image1", .z = image1, .cmap = "gray"});
        PyPlot::Plot({.name = "Image2", .z = image2, .cmap = "gray"});
      }
      else
      {
        Plot::Plot({.name = "Image1", .z = image1, .cmap = "gray"});
        Plot::Plot({.name = "Image2", .z = image2, .cmap = "gray"});
      }
    }

    LOG_PROGRESS(static_cast<f32>(++iprogress) / dataset.imagePairs.size());
  }

  if (mQuanT < 1)
  {
    accuracyPC = QuantileFilter<f64>(accuracyPC / dataset.imageCount, 0, mQuanT);
    accuracyPCS = QuantileFilter<f64>(accuracyPCS / dataset.imageCount, 0, mQuanT);
    accuracyIPC = QuantileFilter<f64>(accuracyIPC / dataset.imageCount, 0, mQuanT);
    accuracyIPCO = QuantileFilter<f64>(accuracyIPCO / dataset.imageCount, 0, mQuanT);
  }

  LOG_SUCCESS("PC average accuracy: {:.3f} ± {:.3f}", Mean<f64>(accuracyPC), Stddev<f64>(accuracyPC));
  LOG_SUCCESS("PCS average accuracy: {:.3f} ± {:.3f}", Mean<f64>(accuracyPCS), Stddev<f64>(accuracyPCS));
  LOG_SUCCESS("IPC average accuracy: {:.3f} ± {:.3f}", Mean<f64>(accuracyIPC), Stddev<f64>(accuracyIPC));
  LOG_SUCCESS("IPCO average accuracy: {:.3f} ± {:.3f}", Mean<f64>(accuracyIPCO), Stddev<f64>(accuracyIPCO));

  PyPlot::PlotCustom(
      "imreg_accuracy", py::dict{"name"_a = "shift error", "x"_a = ColMeans<f64>(refShiftsX), "pc_error"_a = ColMeans<f64>(accuracyPC), "pc_stddev"_a = ColStddevs<f64>(accuracyPC),
                            "pcs_error"_a = ColMeans<f64>(accuracyPCS), "pcs_stddev"_a = ColStddevs<f64>(accuracyPCS), "ipc_error"_a = ColMeans<f64>(accuracyIPC),
                            "ipc_stddev"_a = ColStddevs<f64>(accuracyIPC), "ipco_error"_a = ColMeans<f64>(accuracyIPCO), "ipco_stddev"_a = ColStddevs<f64>(accuracyIPCO)});

  Plot::Plot({.name = "shift error",
      .x = ColMeans<f64>(refShiftsX),
      .ys = {ColMeans<f64>(accuracyPC), ColMeans<f64>(accuracyPCS), ColMeans<f64>(accuracyIPC), ColMeans<f64>(accuracyIPCO)},
      .ylabels = {"pc", "pcs", "ipc", "ipco"},
      .xlabel = "reference shift x [px]",
      .ylabel = "error [px]"});

  static constexpr auto xlabel = "reference shift x [px]";
  static constexpr auto ylabel = "reference shift y [px]";
  const auto [xmin, xmax] = MinMax(refShiftsX);
  const auto [ymin, ymax] = MinMax(refShiftsY);

  Plot::Plot({.name = "PC accuracy", .z = accuracyPC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  Plot::Plot({.name = "PCS accuracy", .z = accuracyPCS, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  Plot::Plot({.name = "IPC accuracy", .z = accuracyIPC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  Plot::Plot({.name = "IPCO accuracy", .z = accuracyIPCO, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});

  PyPlot::Plot({.name = "PC accuracy", .z = accuracyPC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  PyPlot::Plot({.name = "PCS accuracy", .z = accuracyPCS, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  PyPlot::Plot({.name = "IPC accuracy", .z = accuracyIPC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  PyPlot::Plot({.name = "IPCO accuracy", .z = accuracyIPCO, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});

  LOG_PROGRESS_RESET;
}
