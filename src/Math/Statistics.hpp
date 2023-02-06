#pragma once
#include "Utils/Crop.hpp"

template <typename T>
inline f64 Mean(const std::vector<T>& vec)
{
  return std::accumulate(vec.begin(), vec.end(), 0.) / vec.size();
}

template <typename T>
inline f64 Stddev(const std::vector<T>& vec)
{
  const f64 m = Mean(vec);
  f64 stdev = 0;
  for (const auto x : vec)
    stdev += Sqr(x - m);
  stdev /= vec.size();
  return std::sqrt(stdev);
}

template <typename T>
inline f64 Median(std::vector<T>& vec)
{
  if (vec.size() % 2 == 0)
  {
    std::partial_sort(vec.begin(), vec.begin() + vec.size() / 2 + 1, vec.end());
    return 0.5 * vec[vec.size() / 2] + 0.5 * vec[vec.size() / 2 - 1];
  }

  std::nth_element(vec.begin(), vec.begin() + vec.size() / 2, vec.end());
  return vec[vec.size() / 2];
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

inline cv::Mat LeastSquares(const cv::Mat& Y, const cv::Mat& X)
{
  if (Y.rows != X.rows)
    throw std::runtime_error("Data count mismatch");

  return (X.t() * X).inv() * X.t() * Y;
}
