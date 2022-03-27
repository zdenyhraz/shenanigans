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
inline cv::Mat LoadUnitFloatImage(const std::string& path)
{
  PROFILE_FUNCTION;
  cv::Mat mat = cv::imread(path, cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
  mat.convertTo(mat, GetMatType<T>());
  cv::normalize(mat, mat, 0, 1, cv::NORM_MINMAX);
  return mat;
}

inline f64 Magnitude(const cv::Point2f& pt)
{
  return sqrt(Sqr(pt.x) + Sqr(pt.y));
}

inline f64 Angle(const cv::Point2f& pt)
{
  return ToDegrees(atan2(pt.y, pt.x));
}

inline std::pair<f64, f64> MinMax(const cv::Mat& mat)
{
  PROFILE_FUNCTION;
  f64 minR, maxR;
  minMaxLoc(mat, &minR, &maxR, nullptr, nullptr);
  return std::make_pair(minR, maxR);
}

inline cv::Mat LeastSquares(const cv::Mat& Y, const cv::Mat& X)
{
  PROFILE_FUNCTION;
  if (Y.rows != X.rows)
    throw std::runtime_error("Data count mismatch");

  return (X.t() * X).inv() * X.t() * Y;
}

inline void Shift(cv::Mat& mat, const cv::Point2d& shift)
{
  PROFILE_FUNCTION;
  cv::Mat T = (cv::Mat_<f64>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
  cv::warpAffine(mat, mat, T, mat.size());
}

inline void Shift(cv::Mat& mat, f64 shiftx, f64 shifty)
{
  PROFILE_FUNCTION;
  Shift(mat, {shiftx, shifty});
}

inline void Rotate(cv::Mat& mat, f64 rot, f64 scale = 1)
{
  PROFILE_FUNCTION;
  cv::Point2d center(mat.cols / 2, mat.rows / 2);
  cv::Mat R = getRotationMatrix2D(center, rot, scale);
  cv::warpAffine(mat, mat, R, mat.size());
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
