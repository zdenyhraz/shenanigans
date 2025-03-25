#include "IPCMeasure.hpp"
#include "IPC.hpp"
#include "CrossCorrelation.hpp"
#include "PhaseCorrelation.hpp"
#include "PhaseCorrelationUpscale.hpp"
#include "Math/Statistics.hpp"
#include "Plot/Plot.hpp"

void IPCMeasure::MeasureAccuracy(const IPC& ipc, const IPC& ipcopt, const std::string& path)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  const auto dataset = LoadImageRegistrationDataset(path);
  const auto iters = dataset.iters;

  cv::Mat refShiftsX = cv::Mat::zeros(iters, iters, GetMatType<double>());
  cv::Mat refShiftsY = cv::Mat::zeros(iters, iters, GetMatType<double>());
  cv::Mat accuracyPC = cv::Mat::zeros(iters, iters, GetMatType<double>());
  cv::Mat accuracyPCS = cv::Mat::zeros(iters, iters, GetMatType<double>());
  cv::Mat accuracyIPC = cv::Mat::zeros(iters, iters, GetMatType<double>());
  cv::Mat accuracyIPCO = cv::Mat::zeros(iters, iters, GetMatType<double>());

  std::atomic<size_t> iprogress = 0;
#pragma omp parallel for
  for (int idx = 0; idx < dataset.imagePairs.size(); ++idx)
  {
    const auto& image1 = dataset.imagePairs[idx].image1;
    const auto& image2 = dataset.imagePairs[idx].image2;
    const auto& shift = dataset.imagePairs[idx].shift;
    const auto row = dataset.imagePairs[idx].row;
    const auto col = dataset.imagePairs[idx].col;

    refShiftsX.at<double>(row, col) = shift.x;
    refShiftsY.at<double>(row, col) = shift.y;
    accuracyPC.at<double>(row, col) += Magnitude(PhaseCorrelation::Calculate(image1, image2) - shift);
    accuracyPCS.at<double>(row, col) += Magnitude(cv::phaseCorrelate(image1, image2) - shift);
    accuracyIPC.at<double>(row, col) += Magnitude(ipc.Calculate(image1, image2) - shift);
    accuracyIPCO.at<double>(row, col) += Magnitude(ipcopt.Calculate(image1, image2) - shift);

    if (idx == 0)
    {
      if (true)
      {
        Plot::Plot({.name = "Image1", .z = image1, .cmap = "gray"});
        Plot::Plot({.name = "Image2", .z = image2, .cmap = "gray"});
      }
      else
      {
        Plot::Plot({.name = "Image1", .z = image1, .cmap = "gray"});
        Plot::Plot({.name = "Image2", .z = image2, .cmap = "gray"});
      }
    }

    LOG_PROGRESS(static_cast<float>(++iprogress) / dataset.imagePairs.size());
  }

  if (mQuanT < 1)
  {
    accuracyPC = QuantileFilter<double>(accuracyPC / dataset.imageCount, 0, mQuanT);
    accuracyPCS = QuantileFilter<double>(accuracyPCS / dataset.imageCount, 0, mQuanT);
    accuracyIPC = QuantileFilter<double>(accuracyIPC / dataset.imageCount, 0, mQuanT);
    accuracyIPCO = QuantileFilter<double>(accuracyIPCO / dataset.imageCount, 0, mQuanT);
  }

  LOG_SUCCESS("PC average accuracy: {:.3f} ± {:.3f}", Mean<double>(accuracyPC), Stddev<double>(accuracyPC));
  LOG_SUCCESS("PCS average accuracy: {:.3f} ± {:.3f}", Mean<double>(accuracyPCS), Stddev<double>(accuracyPCS));
  LOG_SUCCESS("IPC average accuracy: {:.3f} ± {:.3f}", Mean<double>(accuracyIPC), Stddev<double>(accuracyIPC));
  LOG_SUCCESS("IPCO average accuracy: {:.3f} ± {:.3f}", Mean<double>(accuracyIPCO), Stddev<double>(accuracyIPCO));

  // PyPlot::PlotCustom(
  //     "imreg_accuracy", py::dict{"name"_a = "shift error", "x"_a = ColMeans<double>(refShiftsX), "pc_error"_a = ColMeans<double>(accuracyPC), "pc_stddev"_a =
  //     ColStddevs<double>(accuracyPC),
  //                           "pcs_error"_a = ColMeans<double>(accuracyPCS), "pcs_stddev"_a = ColStddevs<double>(accuracyPCS), "ipc_error"_a = ColMeans<double>(accuracyIPC),
  //                           "ipc_stddev"_a = ColStddevs<double>(accuracyIPC), "ipco_error"_a = ColMeans<double>(accuracyIPCO), "ipco_stddev"_a =
  //                           ColStddevs<double>(accuracyIPCO)});

  Plot::Plot({.name = "shift error",
      .x = ColMeans<double>(refShiftsX),
      .ys = {ColMeans<double>(accuracyPC), ColMeans<double>(accuracyPCS), ColMeans<double>(accuracyIPC), ColMeans<double>(accuracyIPCO)},
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

  Plot::Plot({.name = "PC accuracy", .z = accuracyPC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  Plot::Plot({.name = "PCS accuracy", .z = accuracyPCS, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  Plot::Plot({.name = "IPC accuracy", .z = accuracyIPC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  Plot::Plot({.name = "IPCO accuracy", .z = accuracyIPCO, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});

  LOG_PROGRESS_RESET;
}
