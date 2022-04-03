#pragma once

template <typename T>
inline consteval i32 GetMatType(i32 channels = 1)
{
  if constexpr (std::is_same_v<T, f32>)
    switch (channels)
    {
    case 1:
      return CV_32F;
    case 2:
      return CV_32FC2;
    case 3:
      return CV_32FC3;
    case 4:
      return CV_32FC4;
    }
  if constexpr (std::is_same_v<T, f64>)
    switch (channels)
    {
    case 1:
      return CV_64F;
    case 2:
      return CV_64FC2;
    case 3:
      return CV_64FC3;
    case 4:
      return CV_64FC4;
    }
}

template <typename T>
inline bool Equal(const cv::Mat& mat1, const cv::Mat& mat2, f64 tolerance = 0.)
{
  PROFILE_FUNCTION;
  if (mat1.size() != mat2.size())
    return false;
  if (mat1.channels() != mat2.channels())
    return false;
  if (mat1.depth() != mat2.depth())
    return false;
  if (mat1.type() != mat2.type())
    return false;

  for (i32 r = 0; r < mat1.rows; ++r)
    for (i32 c = 0; c < mat1.cols; ++c)
      if (static_cast<f64>(std::abs(mat1.at<T>(r, c) - mat2.at<T>(r, c))) > tolerance)
        return false;

  return true;
}
