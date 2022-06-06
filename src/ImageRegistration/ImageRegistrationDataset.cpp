#include "ImageRegistrationDataset.hpp"
#include "IPC.hpp"
#include "Filtering/Noise.hpp"

std::vector<ImagePair> CreateImagePairs(const IPC& ipc, const std::vector<cv::Mat>& images, cv::Point2d maxShift, cv::Point2d shiftOffset1,
    cv::Point2d shiftOffset2, i32 iters, f64 noiseStddev, f32* progress)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  if (maxShift.x <= 0 and maxShift.y <= 0)
    throw std::runtime_error(fmt::format("Invalid max shift ({})", maxShift));
  if (iters < 1)
    throw std::runtime_error(fmt::format("Invalid iters per image ({})", iters));

  if (const auto badimage = std::find_if(images.begin(), images.end(),
          [&](const auto& image) { return image.rows < ipc.GetRows() + maxShift.y or image.cols < ipc.GetCols() + maxShift.x; });
      badimage != images.end())
    throw std::runtime_error(fmt::format("Input image is too small for specified IPC window size & max shift ratio ([{},{}] < [{},{}])",
        badimage->rows, badimage->cols, ipc.GetRows() + maxShift.y, ipc.GetCols() + maxShift.x));

  std::vector<ImagePair> imagePairs;
  imagePairs.reserve(images.size() * iters * iters);
  std::atomic<i32> progressidx = 0;
  i32 imageidx = 0;
  for (const auto& image : images)
  {
    LOG_DEBUG("Creating {} image pairs from image {}/{} ...", iters * iters, ++imageidx, images.size());

    cv::Mat image1 = RoiCropMid(Shifted(image, shiftOffset1), ipc.GetCols(), ipc.GetRows());
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
        cv::Mat image2 = RoiCropMid(Shifted(image, shiftOffset1 + shift), ipc.GetCols(), ipc.GetRows());
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

ImageRegistrationDataset LoadImageRegistrationDataset(const std::string& path)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  const auto datasetPath = std::filesystem::weakly_canonical(path).string();
  const auto jsonPath = fmt::format("{}/dataset.json", datasetPath);

  if (not std::filesystem::exists(std::filesystem::weakly_canonical(datasetPath)))
    throw std::invalid_argument(fmt::format("Dataset directory {} does not exist", datasetPath));

  if (not std::filesystem::exists(jsonPath))
    throw std::invalid_argument(fmt::format("Dataset file {} does not exist", jsonPath));

  std::ifstream file(jsonPath);
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
    const i32 row = j["coords"][idx][0];
    const i32 col = j["coords"][idx][1];

    dataset.imagePairs.emplace_back(
        LoadUnitFloatImage<IPC::Float>(path1), LoadUnitFloatImage<IPC::Float>(path2), cv::Point2d(shift.first, shift.second), row, col);
  }

  return dataset;
}

void GenerateImageRegistrationDataset(
    const IPC& ipc, const std::string& path, const std::string& saveDir, i32 iters, f64 maxShiftAbs, f64 noiseStddev, f32* progress)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  auto images = LoadImages<IPC::Float>(path);
  const auto maxShift = cv::Point2d(maxShiftAbs, maxShiftAbs);
  const auto shiftOffset1 = cv::Point2i(3, 3);
  const auto shiftOffset2 = cv::Point2i(0.1 * ipc.GetCols(), 0.1 * ipc.GetRows());
  for (auto& image : images)
    image = RoiCropMid(image, ipc.GetCols() + 2. * (maxShift.x + shiftOffset1.x + shiftOffset2.x + 1),
        ipc.GetRows() + 2. * (maxShift.y + shiftOffset1.y + shiftOffset2.y + 1));

  auto imagePairs = CreateImagePairs(ipc, images, maxShift, shiftOffset1, shiftOffset2, iters, noiseStddev, progress);
  const auto datasetDir = fmt::format(
      "{}/imreg_dataset_{}x{}_{}i_{}ns", std::filesystem::weakly_canonical(saveDir).string(), ipc.GetCols(), ipc.GetRows(), iters, noiseStddev);

  if (not std::filesystem::exists(datasetDir))
    std::filesystem::create_directory(datasetDir);

  std::vector<std::string> image1Paths, image2Paths;
  std::vector<std::pair<f64, f64>> shifts;
  std::vector<std::pair<i32, i32>> coords;
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
    coords.emplace_back(imagePair.row, imagePair.col);

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
  datasetJson["coords"] = coords;
  std::ofstream file(fmt::format("{}/dataset.json", datasetDir));
  file << datasetJson;

  if (progress)
    *progress = 0;
}
