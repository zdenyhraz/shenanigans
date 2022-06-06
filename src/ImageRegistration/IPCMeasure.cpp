#include "IPCMeasure.hpp"
#include "IPC.hpp"
#include "CrossCorrelation.hpp"
#include "PhaseCorrelation.hpp"
#include "PhaseCorrelationUpscale.hpp"

void IPCMeasure::MeasureAccuracy(const IPC& ipc, const IPC& ipcopt, const std::string& path, f32* progress)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  const auto dataset = LoadImageRegistrationDataset(path);
  const auto iters = dataset.iters;

  cv::Mat refShiftsX = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat refShiftsY = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat accuracyCC = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat accuracyPC = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat accuracyPCS = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat accuracyIPC = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat accuracyIPCO = cv::Mat::zeros(iters, iters, GetMatType<f64>());

  std::atomic<i32> idx = 0;
#pragma omp parallel for
  for (const auto& imagePair : dataset.imagePairs)
  {
    const auto& image1 = imagePair.image1;
    const auto& image2 = imagePair.image2;
    const auto& shift = imagePair.shift;
    const auto row = imagePair.row;
    const auto col = imagePair.col;

    refShiftsX.at<f64>(row, col) = shift.x;
    refShiftsY.at<f64>(row, col) = shift.y;
    accuracyCC.at<f64>(row, col) += Magnitude(CrossCorrelation::Calculate(image1, image2) - shift);
    accuracyPC.at<f64>(row, col) += Magnitude(PhaseCorrelation::Calculate(image1, image2) - shift);
    accuracyPCS.at<f64>(row, col) += Magnitude(cv::phaseCorrelate(image1, image2) - shift);
    accuracyIPC.at<f64>(row, col) += Magnitude(ipc.Calculate(image1, image2) - shift);
    accuracyIPCO.at<f64>(row, col) += Magnitude(ipcopt.Calculate(image1, image2) - shift);

    if (idx == 0)
    {
      Plot2D("Image1", {.z = image1, .cmap = Gray});
      Plot2D("Image2", {.z = image2, .cmap = Gray});
    }

    if (progress)
      *progress = static_cast<f32>(idx) / dataset.imagePairs.size();

    ++idx;
  }

  if (mQuanT < 1)
  {
    accuracyCC = QuantileFilter<f64>(accuracyCC / dataset.imageCount, 0, mQuanT);
    accuracyPC = QuantileFilter<f64>(accuracyPC / dataset.imageCount, 0, mQuanT);
    accuracyPCS = QuantileFilter<f64>(accuracyPCS / dataset.imageCount, 0, mQuanT);
    accuracyIPC = QuantileFilter<f64>(accuracyIPC / dataset.imageCount, 0, mQuanT);
    accuracyIPCO = QuantileFilter<f64>(accuracyIPCO / dataset.imageCount, 0, mQuanT);
  }

  LOG_SUCCESS("CC average accuracy: {:.3f} ± {:.3f}", Mean<f64>(accuracyCC), Stddev<f64>(accuracyCC));
  LOG_SUCCESS("PC average accuracy: {:.3f} ± {:.3f}", Mean<f64>(accuracyPC), Stddev<f64>(accuracyPC));
  LOG_SUCCESS("PCS average accuracy: {:.3f} ± {:.3f}", Mean<f64>(accuracyPCS), Stddev<f64>(accuracyPCS));
  LOG_SUCCESS("IPC average accuracy: {:.3f} ± {:.3f}", Mean<f64>(accuracyIPC), Stddev<f64>(accuracyIPC));
  LOG_SUCCESS("IPCO average accuracy: {:.3f} ± {:.3f}", Mean<f64>(accuracyIPCO), Stddev<f64>(accuracyIPCO));

  PyPlot::Plot("shift error", "imreg_accuracy",
      py::dict{"x"_a = ColMeans<f64>(refShiftsX), "pc_error"_a = ColMeans<f64>(accuracyPC), "pc_stddev"_a = ColStddevs<f64>(accuracyPC),
          "pcs_error"_a = ColMeans<f64>(accuracyPCS), "pcs_stddev"_a = ColStddevs<f64>(accuracyPCS), "ipc_error"_a = ColMeans<f64>(accuracyIPC),
          "ipc_stddev"_a = ColStddevs<f64>(accuracyIPC), "ipco_error"_a = ColMeans<f64>(accuracyIPCO),
          "ipco_stddev"_a = ColStddevs<f64>(accuracyIPCO)});

  Plot1D("shift error", {.x = ColMeans<f64>(refShiftsX),
                            .ys = {ColMeans<f64>(accuracyPC), ColMeans<f64>(accuracyPCS), ColMeans<f64>(accuracyIPC), ColMeans<f64>(accuracyIPCO)},
                            .ylabels = {"pc", "pcs", "ipc", "ipco"},
                            .xlabel = "reference shift x [px]",
                            .ylabel = "error [px]"});

  static constexpr auto xlabel = "reference shift x [px]";
  static constexpr auto ylabel = "reference shift y [px]";
  const auto [xmin, xmax] = MinMax(refShiftsX);
  const auto [ymin, ymax] = MinMax(refShiftsY);

  Plot2D("CC accuracy", {.z = accuracyCC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  Plot2D("PC accuracy", {.z = accuracyPC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  Plot2D("PCS accuracy", {.z = accuracyPCS, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  Plot2D("IPC accuracy", {.z = accuracyIPC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  Plot2D("IPCO accuracy", {.z = accuracyIPCO, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});

  PyPlot::Plot("ref shifts x", {.z = refShiftsX, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  PyPlot::Plot("ref shifts y", {.z = refShiftsY, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  PyPlot::Plot("CC accuracy", {.z = accuracyCC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  PyPlot::Plot("PC accuracy", {.z = accuracyPC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  PyPlot::Plot("PCS accuracy", {.z = accuracyPCS, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  PyPlot::Plot("IPC accuracy", {.z = accuracyIPC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  PyPlot::Plot("IPCO accuracy", {.z = accuracyIPCO, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});

  if (progress)
    *progress = 0;
}
