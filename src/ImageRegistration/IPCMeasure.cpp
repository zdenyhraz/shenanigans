#include "IPCMeasure.hpp"
#include "IPC.hpp"
#include "Filtering/Noise.hpp"
#include "CrossCorrelation.hpp"
#include "PhaseCorrelation.hpp"
#include "PhaseCorrelationUpscale.hpp"

std::vector<ImagePair> IPCMeasure::CreateImagePairs(const IPC& ipc, const std::vector<cv::Mat>& images, cv::Point2d maxShift,
    cv::Point2d shiftOffset1, cv::Point2d shiftOffset2, i32 iters, f64 noiseStddev, f32* progress)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  if (maxShift.x <= 0 and maxShift.y <= 0)
    throw std::runtime_error(fmt::format("Invalid max shift ({})", maxShift));
  if (iters < 1)
    throw std::runtime_error(fmt::format("Invalid iters per image ({})", iters));

  if (const auto badimage = std::find_if(images.begin(), images.end(),
          [&](const auto& image) { return image.rows < ipc.mRows + maxShift.y or image.cols < ipc.mCols + maxShift.x; });
      badimage != images.end())
    throw std::runtime_error(fmt::format("Input image is too small for specified IPC window size & max shift ratio ([{},{}] < [{},{}])",
        badimage->rows, badimage->cols, ipc.mRows + maxShift.y, ipc.mCols + maxShift.x));

  std::vector<ImagePair> imagePairs;
  imagePairs.reserve(images.size() * iters * iters);
  std::atomic<i32> progressidx = 0;
  for (const auto& image : images)
  {
    static i32 i = 0;
    LOG_DEBUG("Creating {} image pairs from image {}/{} ...", iters * iters, ++i, images.size());

    cv::Mat image1 = RoiCropMid(Shifted(image, shiftOffset1), ipc.mCols, ipc.mRows);
    AddNoise<IPC::Float>(image1, noiseStddev);

#pragma omp parallel for
    for (i32 row = 0; row < iters; ++row)
    {
      for (i32 col = 0; col < iters; ++col)
      {
        if (progress)
          *progress = static_cast<f32>(++progressidx) / (images.size() * iters * iters);

        const auto shift =
            cv::Point2d(maxShift.x * (-1.0 + 2.0 * col / (iters - 1)), maxShift.y * (-1.0 + 2.0 * (iters - 1 - row) / (iters - 1))) + shiftOffset2;
        cv::Mat image2 = RoiCropMid(Shifted(image, shiftOffset1 + shift), ipc.mCols, ipc.mRows);
        AddNoise<IPC::Float>(image2, noiseStddev);
#pragma omp critical
        imagePairs.emplace_back(image1, image2, shift, row, col);
      }
    }
  }

  if (progress)
    *progress = 0;
  return imagePairs;
}

void IPCMeasure::MeasureAccuracy(
    const IPC& ipc, const IPC& ipcopt, const std::string& path, i32 iters, f64 maxShiftAbs, f64 noiseStddev, f32* progress)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  cv::Mat refShiftsX = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat refShiftsY = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat accuracyCC = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat accuracyPC = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat accuracyPCS = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat accuracyIPC = cv::Mat::zeros(iters, iters, GetMatType<f64>());
  cv::Mat accuracyIPCO = cv::Mat::zeros(iters, iters, GetMatType<f64>());

  const auto maxShift = cv::Point2d(maxShiftAbs, maxShiftAbs);
  const auto shiftOffset1 = cv::Point2d(0.3, 0.6);
  const auto shiftOffset2 = cv::Point2d(0.1 * ipc.mCols, 0.1 * ipc.mRows);
  const auto images = LoadImages<IPC::Float>(path);

  if (false) // downsize for speedup
    for (auto& image : images)
      cv::resize(image, image,
          cv::Size(ipc.mCols + 2. * (maxShift.x + shiftOffset1.x + shiftOffset2.x + 1),
              ipc.mRows + 2. * (maxShift.y + shiftOffset1.y + shiftOffset2.y + 1)));

  const auto imagePairs = CreateImagePairs(ipc, images, maxShift, shiftOffset1, shiftOffset2, iters, noiseStddev, progress);
  LOG_DEBUG("Measuring {}% image registration accuracy on {} image pairs from {}...", mQuanT * 100, imagePairs.size(), path);

  std::atomic<i32> idx = 0;
#pragma omp parallel for
  for (const auto& imagePair : imagePairs)
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
      Plot2D("Image1", {.z = image1});
      Plot2D("Image2", {.z = image2});
    }

    if (progress)
      *progress = static_cast<f32>(idx + 1) / imagePairs.size();

    ++idx;
  }

  if (mQuanT < 1)
  {
    accuracyCC = QuantileFilter<f64>(accuracyCC / images.size(), 0, mQuanT);
    accuracyPC = QuantileFilter<f64>(accuracyPC / images.size(), 0, mQuanT);
    accuracyPCS = QuantileFilter<f64>(accuracyPCS / images.size(), 0, mQuanT);
    accuracyIPC = QuantileFilter<f64>(accuracyIPC / images.size(), 0, mQuanT);
    accuracyIPCO = QuantileFilter<f64>(accuracyIPCO / images.size(), 0, mQuanT);
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
  const auto [ymin, ymax] = MinMax(refShiftsX);

  Plot2D("CC accuracy", {.z = accuracyCC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  Plot2D("PC accuracy", {.z = accuracyPC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  Plot2D("PCS accuracy", {.z = accuracyPCS, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  Plot2D("IPC accuracy", {.z = accuracyIPC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  Plot2D("IPCO accuracy", {.z = accuracyIPCO, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});

  PyPlot::Plot("PC accuracy", {.z = accuracyPC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  PyPlot::Plot("PCS accuracy", {.z = accuracyPCS, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  PyPlot::Plot("IPC accuracy", {.z = accuracyIPC, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});
  PyPlot::Plot("IPCO accuracy", {.z = accuracyIPCO, .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax, .xlabel = xlabel, .ylabel = ylabel});

  if (progress)
    *progress = 0;
}
