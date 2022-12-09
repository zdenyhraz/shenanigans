#pragma once

inline bool IsImagePath(const std::string& path)
{
  return path.ends_with(".png") or path.ends_with(".PNG") or path.ends_with(".jpg") or path.ends_with(".JPG") or path.ends_with(".jpeg") or path.ends_with(".JPEG");
}

template <typename T>
inline cv::Mat LoadUnitFloatImage(const std::filesystem::path path)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;
  LOG_DEBUG("Loading image {}...", path.string());
  cv::Mat mat = cv::imread(path.string(), cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
  if (mat.empty()) [[unlikely]]
    throw std::runtime_error(fmt::format("Image '{}' not found", path.string()));
  mat.convertTo(mat, GetMatType<T>());
  cv::normalize(mat, mat, 0, 1, cv::NORM_MINMAX);
  return mat;
}

template <typename T>
inline std::vector<cv::Mat> LoadImages(const std::string& dirpath)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;
  LOG_DEBUG("Loading images from {}...", dirpath);

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
