#pragma once

inline cv::Mat LoadImage(const std::filesystem::path& path)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;
  LOG_DEBUG("Loading image {}", path.string());
  if (not std::filesystem::exists(path))
    throw std::invalid_argument(fmt::format("File {} does not exist", path.string()));
  cv::Mat mat = cv::imread(path.string(), cv::IMREAD_ANYDEPTH);
  return mat;
}

template <typename T>
inline cv::Mat LoadUnitFloatImage(const std::filesystem::path& path)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;
  LOG_DEBUG("Loading image {}", path.string());
  if (not std::filesystem::exists(path))
    throw std::invalid_argument(fmt::format("File {} does not exist", path.string()));
  cv::Mat mat = cv::imread(path.string(), cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
  mat.convertTo(mat, GetMatType<T>());
  cv::normalize(mat, mat, 0, 1, cv::NORM_MINMAX);
  return mat;
}

template <typename T>
inline std::vector<cv::Mat> LoadImages(const std::string& dirpath)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;
  LOG_DEBUG("Loading images from {}", dirpath);

  if (!std::filesystem::is_directory(dirpath))
    throw std::runtime_error(fmt::format("'{}' is not a valid directory", dirpath));

  std::vector<cv::Mat> images;
  for (const auto& entry : std::filesystem::directory_iterator(dirpath))
  {
    const auto filepath = entry.path().string();

    if (not IsImagePath(filepath))
    {
      LOG_WARNING("Directory '{}' contains a non-image file '{}'", dirpath, filepath);
      continue;
    }

    images.push_back(LoadUnitFloatImage<T>(filepath));
    LOG_DEBUG("Loaded image '{}'", filepath);
  }

  return images;
}

inline void ResizeImages(std::vector<cv::Mat>& images, cv::Size size)
{
  for (auto& image : images)
    cv::resize(image, image, size);
}
