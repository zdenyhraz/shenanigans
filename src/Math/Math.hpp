#pragma once

template <typename T = f64>
inline auto Zerovect(i32 N, T value = 0.)
{
  return std::vector<T>(N, value);
}

template <typename T = f64>
inline auto Zerovect2(i32 N, i32 M, T value = 0.)
{
  return std::vector<std::vector<T>>(N, Zerovect(M, value));
}

template <typename T>
inline std::vector<T> Iota(T first, usize size)
{
  std::vector<T> vec(size);
  std::iota(vec.begin(), vec.end(), first);
  return vec;
}

constexpr f64 ToRadians(f64 degrees)
{
  return degrees / 360. * 2 * std::numbers::pi;
}

constexpr f64 ToDegrees(f64 radians)
{
  return radians * 360. / (2 * std::numbers::pi);
}

template <typename T>
inline constexpr T Sqr(T x)
{
  return x * x;
}

template <typename T>
constexpr std::vector<T> Linspace(f64 start, f64 end, usize n)
{
  std::vector<T> vec(n);
  for (i32 i = 0; i < n; ++i)
    vec[i] = start + static_cast<f64>(i) / (n - 1) * (end - start);
  return vec;
}

inline i32 GetNearestOdd(i32 value)
{
  return value % 2 == 0 ? value + 1 : value;
}

inline i32 GetNearestEven(i32 value)
{
  return value % 2 != 0 ? value + 1 : value;
}

template <typename T>
inline f64 Magnitude(const cv::Point_<T>& pt)
{
  return std::sqrt(std::pow(pt.x, 2) + std::pow(pt.y, 2));
}

template <typename T>
inline f64 Angle(const cv::Point_<T>& pt)
{
  return std::atan2(pt.y, pt.x);
}

inline std::pair<f64, f64> MinMax(const cv::Mat& mat)
{
  f64 minR, maxR;
  cv::minMaxLoc(mat, &minR, &maxR, nullptr, nullptr);
  return {minR, maxR};
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
