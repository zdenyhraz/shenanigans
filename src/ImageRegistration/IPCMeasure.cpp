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
  i32 imageidx = 0;
  for (const auto& image : images)
  {
    LOG_DEBUG("Creating {} image pairs from image {}/{} ...", iters * iters, ++imageidx, images.size());

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

  // monotonically increasing x shift for plots
  std::sort(
      imagePairs.begin(), imagePairs.end(), [](const auto& imagePair1, const auto& imagePair2) { return imagePair1.shift.x < imagePair2.shift.x; });

  if (progress)
    *progress = 0;
  return imagePairs;
}

ImageRegistrationDataset IPCMeasure::LoadImageRegistrationDataset(const std::string& path)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  std::ifstream file(fmt::format("{}/dataset.json", std::filesystem::weakly_canonical(path).string()));
  json::json j;
  file >> j;

  ImageRegistrationDataset dataset;
  dataset.rows = j["rows"];
  dataset.cols = j["cols"];
  dataset.imageCount = j["imageCount"];
  dataset.iters = j["iters"];
  dataset.maxShift = j["maxShift"];
  dataset.noiseStddev = j["noiseStddev"];

  for (i32 idx = 0; idx < dataset.imageCount * dataset.iters * dataset.iters; ++idx)
  {
    const std::string path1 = j["image1Paths"][idx];
    const std::string path2 = j["image2Paths"][idx];
    const std::pair<f64, f64> shift = j["shifts"][idx];
    const i32 row = idx / dataset.iters;
    const i32 col = idx % dataset.iters;

    dataset.imagePairs.emplace_back(
        LoadUnitFloatImage<IPC::Float>(path1), LoadUnitFloatImage<IPC::Float>(path2), cv::Point2d(shift.first, shift.second), row, col);
  }

  return dataset;
}

void IPCMeasure::GenerateRegistrationDataset(
    const IPC& ipc, const std::string& path, const std::string& saveDir, i32 iters, f64 maxShiftAbs, f64 noiseStddev, f32* progress)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  auto images = LoadImages<IPC::Float>(path);
  const auto maxShift = cv::Point2d(maxShiftAbs, maxShiftAbs);
  const auto shiftOffset1 = cv::Point2i(3, 3);
  const auto shiftOffset2 = cv::Point2i(0.1 * ipc.mCols, 0.1 * ipc.mRows);
  for (auto& image : images)
    image = RoiCropMid(image, ipc.mCols + 2. * (maxShift.x + shiftOffset1.x + shiftOffset2.x + 1),
        ipc.mRows + 2. * (maxShift.y + shiftOffset1.y + shiftOffset2.y + 1));

  auto imagePairs = CreateImagePairs(ipc, images, maxShift, shiftOffset1, shiftOffset2, iters, noiseStddev, progress);
  const auto datasetDir = fmt::format(
      "{}/imreg_dataset_{}x{}_{}i_{}ns", std::filesystem::weakly_canonical(saveDir).string(), ipc.GetCols(), ipc.GetRows(), iters, noiseStddev);

  if (not std::filesystem::exists(datasetDir))
    std::filesystem::create_directory(datasetDir);

  std::vector<std::string> image1Paths, image2Paths;
  std::vector<std::pair<f64, f64>> shifts;
  i32 idx = 0;
  for (auto& imagePair : imagePairs)
  {
    auto& image1 = imagePair.image1;
    auto& image2 = imagePair.image2;
    const auto shift = imagePair.shift;
    const auto path1 = fmt::format("{}/pair{}_a.png", datasetDir, idx);
    const auto path2 = fmt::format("{}/pair{}_b.png", datasetDir, idx);

    image1.convertTo(image1, CV_16U, 65535);
    image2.convertTo(image2, CV_16U, 65535);

    LOG_DEBUG("Saving image pair {} & {}...", path1, path2);
    cv::imwrite(path1, image1);
    cv::imwrite(path2, image2);

    image1Paths.emplace_back(path1);
    image2Paths.emplace_back(path2);
    shifts.emplace_back(shift.x, shift.y);

    if (progress)
      *progress = static_cast<f32>(++idx) / imagePairs.size();
  }

  LOG_DEBUG("Generating dataset json file...");
  json::json datasetJson;
  datasetJson["rows"] = ipc.GetRows();
  datasetJson["cols"] = ipc.GetCols();
  datasetJson["imageCount"] = images.size();
  datasetJson["iters"] = iters;
  datasetJson["maxShift"] = maxShiftAbs;
  datasetJson["noiseStddev"] = noiseStddev;
  datasetJson["image1Paths"] = image1Paths;
  datasetJson["image2Paths"] = image2Paths;
  datasetJson["shifts"] = shifts;
  std::ofstream file(fmt::format("{}/dataset.json", datasetDir));
  file << datasetJson;

  if (progress)
    *progress = 0;
}

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
  LOG_DEBUG("Plots shifts x limits: [{},{}]", xmin, xmax);
  LOG_DEBUG("Plots shifts y limits: [{},{}]", ymin, ymax);

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
