#pragma once

template <typename T = double>
inline auto Zerovect(int N, T value = 0.)
{
  return std::vector<T>(N, value);
}

template <typename T = double>
inline auto Zerovect2(int N, int M, T value = 0.)
{
  return std::vector<std::vector<T>>(N, Zerovect(M, value));
}

template <typename T>
inline std::vector<T> Iota(T first, size_t size)
{
  std::vector<T> vec(size);
  std::iota(vec.begin(), vec.end(), first);
  return vec;
}

constexpr double ToRadians(double degrees)
{
  return degrees / 360. * 2 * std::numbers::pi;
}

constexpr double ToDegrees(double radians)
{
  return radians * 360. / (2 * std::numbers::pi);
}

template <typename T>
inline constexpr T Sqr(T x)
{
  return x * x;
}

template <typename T>
constexpr std::vector<T> Linspace(double start, double end, size_t n)
{
  std::vector<T> vec(n);
  for (int i = 0; i < n; ++i)
    vec[i] = start + static_cast<double>(i) / (n - 1) * (end - start);
  return vec;
}

inline int GetNearestOdd(int value)
{
  return value % 2 ? value : value + 1;
}

inline int GetNearestEven(int value)
{
  return value % 2 ? value + 1 : value;
}

template <typename T>
inline constexpr int GetMatType(int channels = 1)
{
  if constexpr (std::is_same_v<T, uint8_t>)
    switch (channels)
    {
    case 1:
      return CV_8U;
    case 2:
      return CV_8UC2;
    case 3:
      return CV_8UC3;
    case 4:
      return CV_8UC4;
    }
  if constexpr (std::is_same_v<T, uint16_t>)
    switch (channels)
    {
    case 1:
      return CV_16U;
    case 2:
      return CV_16UC2;
    case 3:
      return CV_16UC3;
    case 4:
      return CV_16UC4;
    }
  if constexpr (std::is_same_v<T, float>)
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
  if constexpr (std::is_same_v<T, double>)
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
  return CV_32F;
}

template <typename T>
inline double Magnitude(const cv::Point_<T>& pt)
{
  return std::sqrt(std::pow(pt.x, 2) + std::pow(pt.y, 2));
}

template <typename T>
inline double Angle(const cv::Point_<T>& pt)
{
  return std::atan2(pt.y, pt.x);
}

inline std::pair<double, double> MinMax(const cv::Mat& mat)
{
  double minR, maxR;
  cv::minMaxLoc(mat, &minR, &maxR, nullptr, nullptr);
  return {minR, maxR};
}

inline double Gaussian(double x, double amp, double mid, double sigma)
{
  return amp * std::exp(-0.5 * std::pow((x - mid) / sigma, 2));
}

template <typename T>
inline cv::Mat Gaussian(int size, double stddev)
{
  cv::Mat mat(size, size, GetMatType<T>());
  for (int row = 0; row < size; ++row)
  {
    auto matp = mat.ptr<T>(row);
    for (int col = 0; col < size; ++col)
    {
      double r = std::sqrt(Sqr(row - size / 2) + Sqr(col - size / 2));
      matp[col] = Gaussian(r, 1, 0, stddev);
    }
  }
  return mat;
}

template <typename T>
inline cv::Mat Kirkl(int rows, int cols, double radius)
{
  cv::Mat mat = cv::Mat(rows, cols, GetMatType<T>());
  const int radsq = Sqr(radius);
  const int rowsh = rows / 2;
  const int colsh = cols / 2;
  for (int r = 0; r < rows; ++r)
  {
    auto matp = mat.ptr<T>(r);
    for (int c = 0; c < cols; ++c)
      matp[c] = (Sqr(r - rowsh) + Sqr(c - colsh)) <= radsq;
  }
  return mat;
}

template <typename T>
inline cv::Mat Kirkl(int size)
{
  return Kirkl<T>(size, size, 0.5 * size);
}

template <typename T>
cv::Mat Butterworth(cv::Size size, double cutoff, int order)
{
  cv::Mat result(size, GetMatType<T>());
  const auto D0 = cutoff * std::sqrt(Sqr(size.height / 2) + Sqr(size.width / 2));
  for (int r = 0; r < size.height; ++r)
  {
    for (int c = 0; c < size.width; ++c)
    {
      const auto D = std::sqrt(Sqr(r - size.height / 2) + Sqr(c - size.width / 2));
      result.at<T>(r, c) = 1 / (1 + std::pow(D / D0, 2 * order));
    }
  }
  return result;
}
