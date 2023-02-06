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

inline f64 Gaussian(f64 x, f64 amp, f64 mid, f64 sigma)
{
  return amp * std::exp(-0.5 * std::pow((x - mid) / sigma, 2));
}

template <typename T>
inline cv::Mat Gaussian(i32 size, f64 stddev)
{
  cv::Mat mat(size, size, GetMatType<T>());
  for (i32 row = 0; row < size; ++row)
  {
    auto matp = mat.ptr<T>(row);
    for (i32 col = 0; col < size; ++col)
    {
      f64 r = std::sqrt(Sqr(row - size / 2) + Sqr(col - size / 2));
      matp[col] = Gaussian(r, 1, 0, stddev);
    }
  }
  return mat;
}

template <typename T>
inline cv::Mat Kirkl(i32 rows, i32 cols, f64 radius)
{
  cv::Mat mat = cv::Mat(rows, cols, GetMatType<T>());
  const i32 radsq = Sqr(radius);
  const i32 rowsh = rows / 2;
  const i32 colsh = cols / 2;
  for (i32 r = 0; r < rows; ++r)
  {
    auto matp = mat.ptr<T>(r);
    for (i32 c = 0; c < cols; ++c)
      matp[c] = (Sqr(r - rowsh) + Sqr(c - colsh)) <= radsq;
  }
  return mat;
}

template <typename T>
inline cv::Mat Kirkl(i32 size)
{
  return Kirkl<T>(size, size, 0.5 * size);
}

template <typename T>
cv::Mat Butterworth(cv::Size size, f64 cutoff, i32 order)
{
  cv::Mat result(size, GetMatType<T>());
  const auto D0 = cutoff * std::sqrt(Sqr(size.height / 2) + Sqr(size.width / 2));
  for (i32 r = 0; r < size.height; ++r)
  {
    for (i32 c = 0; c < size.width; ++c)
    {
      const auto D = std::sqrt(Sqr(r - size.height / 2) + Sqr(c - size.width / 2));
      result.at<T>(r, c) = 1 / (1 + std::pow(D / D0, 2 * order));
    }
  }
  return result;
}
