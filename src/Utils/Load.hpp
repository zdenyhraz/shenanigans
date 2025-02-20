#pragma once

inline cv::Mat LoadImage(const std::filesystem::path& path)
{
  PROFILE_FUNCTION;
  if (not std::filesystem::is_regular_file(path))
    throw std::invalid_argument(fmt::format("File {} does not exist", path.string()));
  return cv::imread(path.string());
}

template <typename T>
inline cv::Mat LoadUnitFloatImage(const std::filesystem::path& path)
{
  PROFILE_FUNCTION;
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
  if (!std::filesystem::is_directory(dirpath))
    throw std::runtime_error(fmt::format("'{}' is not a valid directory", dirpath));

  std::vector<cv::Mat> images;
  for (const auto& entry : std::filesystem::directory_iterator(dirpath))
  {
    images.push_back(LoadUnitFloatImage<T>(entry.path()));
    LOG_DEBUG("Loaded image '{}'", entry.path().string());
  }

  return images;
}

inline void ResizeImages(std::vector<cv::Mat>& images, cv::Size size)
{
  for (auto& image : images)
    cv::resize(image, image, size);
}
