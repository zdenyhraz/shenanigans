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
  normalize(mat, mat, 0, 1, cv::NORM_MINMAX);
  return mat;
}

inline f64 magnitude(const cv::Point2f& pt)
{
  return sqrt(sqr(pt.x) + sqr(pt.y));
}

inline f64 angle(const cv::Point2f& pt)
{
  return toDegrees(atan2(pt.y, pt.x));
}

inline std::pair<f64, f64> minMaxMat(const cv::Mat& sourceimg)
{
  f64 minR, maxR;
  minMaxLoc(sourceimg, &minR, &maxR, nullptr, nullptr);
  return std::make_pair(minR, maxR);
}

inline std::string to_string(const cv::Point2d& point)
{
  return std::string("[" + std::to_string(point.x) + "," + std::to_string(point.y) + "]");
}

inline cv::Point2d findCentroid(const cv::Mat& mat)
{
  f32 m00 = 0, m10 = 0, m01 = 0;
  for (i32 r = 0; r < mat.rows; ++r)
  {
    const auto matp = mat.ptr<f32>(r);
    for (i32 c = 0; c < mat.cols; ++c)
    {
      const auto val = matp[c];
      m00 += val;
      m10 += val * c;
      m01 += val * r;
    }
  }

  return cv::Point2d(static_cast<f64>(m10) / m00, static_cast<f64>(m01) / m00);
}

inline cv::Point2d median(std::vector<cv::Point2d>& vec)
{
  // function changes the vec order, watch out
  std::sort(vec.begin(), vec.end(), [](cv::Point2d a, cv::Point2d b) { return a.x < b.x; });
  return vec[vec.size() / 2];
}

inline cv::Point2f mean(const std::vector<cv::Point2f>& vec)
{
  cv::Point2f mean(0, 0);
  for (auto& x : vec)
    mean += x;
  return mean * (1. / vec.size());
}

inline void Shift(cv::Mat& image, const cv::Point2f& shift)
{
  cv::Mat T = (cv::Mat_<f32>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
  warpAffine(image, image, T, image.size());
}

inline void Shift(cv::Mat& image, f32 shiftx, f32 shifty)
{
  Shift(image, {shiftx, shifty});
}

inline void Rotate(cv::Mat& image, f32 rot, f32 scale = 1)
{
  cv::Point2f center((f32)image.cols / 2, (f32)image.rows / 2);
  cv::Mat R = getRotationMatrix2D(center, rot, scale);
  warpAffine(image, image, R, image.size());
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
