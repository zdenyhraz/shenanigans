#pragma once
#include "Crop.hpp"

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

template <typename T>
inline f64 Mean(const cv::Mat& mat)
{
  f64 mean = 0;
  for (i32 r = 0; r < mat.rows; ++r)
  {
    auto matp = mat.ptr<T>(r);
    for (i32 c = 0; c < mat.cols; ++c)
      mean += matp[c];
  }
  return mean / mat.rows / mat.cols;
}

template <typename T>
inline f64 Stddev(const cv::Mat& mat)
{
  f64 mean = Mean<T>(mat);
  f64 stddev = 0;
  for (i32 r = 0; r < mat.rows; ++r)
  {
    auto matp = mat.ptr<T>(r);
    for (i32 c = 0; c < mat.cols; ++c)
      stddev += std::pow(matp[c] - mean, 2);
  }
  return std::sqrt(stddev / mat.rows / mat.cols);
}

template <typename T>
inline std::vector<f64> ColMeans(const cv::Mat& mat)
{
  std::vector<f64> means(mat.cols);

  for (i32 c = 0; c < mat.cols; ++c)
    means[c] = Mean<T>(RoiCropRef(mat, c, mat.rows / 2, 1, mat.rows));

  return means;
}

template <typename T>
inline std::vector<f64> ColStddevs(const cv::Mat& mat)
{
  std::vector<f64> stddevs(mat.cols);

  for (i32 c = 0; c < mat.cols; ++c)
    stddevs[c] = Stddev<T>(RoiCropRef(mat, c, mat.rows / 2, 1, mat.rows));

  return stddevs;
}

template <typename T>
inline cv::Mat QuantileFilter(const cv::Mat& mat, f64 quantileB, f64 quantileT)
{
  if (quantileB <= 0 and quantileT >= 1)
    return mat.clone();

  cv::Mat matq = mat.clone();
  matq.convertTo(matq, GetMatType<T>());
  std::vector<T> values(matq.rows * matq.cols);

  for (i32 r = 0; r < matq.rows; ++r)
    for (i32 c = 0; c < matq.cols; ++c)
      values[r * matq.cols + c] = matq.at<T>(r, c);

  std::sort(values.begin(), values.end());
  const auto valMin = values[quantileB * (values.size() - 1)];
  const auto valMax = values[quantileT * (values.size() - 1)];

  for (i32 r = 0; r < matq.rows; ++r)
    for (i32 c = 0; c < matq.cols; ++c)
      matq.at<T>(r, c) = std::clamp(matq.at<T>(r, c), valMin, valMax);

  return matq;
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

inline cv::Mat LeastSquares(const cv::Mat& Y, const cv::Mat& X)
{
  if (Y.rows != X.rows)
    throw std::runtime_error("Data count mismatch");

  return (X.t() * X).inv() * X.t() * Y;
}

inline std::pair<f64, f64> MinMax(const cv::Mat& mat)
{
  f64 minR, maxR;
  cv::minMaxLoc(mat, &minR, &maxR, nullptr, nullptr);
  return {minR, maxR};
}

template <typename T>
inline cv::Mat Hanning(cv::Size size)
{
  cv::Mat mat;
  cv::createHanningWindow(mat, size, GetMatType<T>());
  return mat;
}
