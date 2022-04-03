#pragma once

inline bool IsImagePath(const std::string& path)
{
  return path.ends_with(".png") or path.ends_with(".PNG") or path.ends_with(".jpg") or path.ends_with(".JPG") or path.ends_with(".jpeg") or path.ends_with(".JPEG");
}

template <typename T>
inline cv::Mat LoadUnitFloatImage(const std::string& path)
{
  PROFILE_FUNCTION;
  cv::Mat mat = cv::imread(path, cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
  mat.convertTo(mat, GetMatType<T>());
  cv::normalize(mat, mat, 0, 1, cv::NORM_MINMAX);
  return mat;
}

template <typename T>
inline std::vector<cv::Mat> LoadImages(const std::string& path)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("LoadImages");
  LOG_INFO("Loading images from '{}'...", path);

  if (!std::filesystem::is_directory(path))
    throw std::runtime_error(fmt::format("Directory '{}' is not a valid directory", path));

  std::vector<cv::Mat> images;
  for (const auto& entry : std::filesystem::directory_iterator(path))
  {
    const auto path = entry.path().string();

    if (not IsImagePath(path))
    {
      LOG_WARNING("Directory contains a non-image file {}", path);
      continue;
    }

    auto image = LoadUnitFloatImage<T>(path);
    images.push_back(image);
    LOG_DEBUG("Loaded image {}", path);
  }

  return images;
}
