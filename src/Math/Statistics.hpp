#pragma once
#include "Utils/Crop.hpp"
#include "Math/Functions.hpp"

template <typename T>
inline double Mean(const std::vector<T>& vec)
{
  return std::accumulate(vec.begin(), vec.end(), 0.) / vec.size();
}

template <typename T>
inline double Stddev(const std::vector<T>& vec)
{
  const double m = Mean(vec);
  double stdev = 0;
  for (const auto x : vec)
    stdev += Sqr(x - m);
  stdev /= vec.size();
  return std::sqrt(stdev);
}

template <typename T>
inline double Median(std::vector<T>& vec)
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
inline double Mean(const cv::Mat& mat)
{
  double mean = 0;
  for (int r = 0; r < mat.rows; ++r)
  {
    auto matp = mat.ptr<T>(r);
    for (int c = 0; c < mat.cols; ++c)
      mean += matp[c];
  }
  return mean / mat.rows / mat.cols;
}

template <typename T>
inline double Stddev(const cv::Mat& mat)
{
  double mean = Mean<T>(mat);
  double stddev = 0;
  for (int r = 0; r < mat.rows; ++r)
  {
    auto matp = mat.ptr<T>(r);
    for (int c = 0; c < mat.cols; ++c)
      stddev += std::pow(matp[c] - mean, 2);
  }
  return std::sqrt(stddev / mat.rows / mat.cols);
}

template <typename T>
inline std::vector<double> ColMeans(const cv::Mat& mat)
{
  std::vector<double> means(mat.cols);

  for (int c = 0; c < mat.cols; ++c)
    means[c] = Mean<T>(RoiCropRef(mat, c, mat.rows / 2, 1, mat.rows));

  return means;
}

template <typename T>
inline std::vector<double> ColStddevs(const cv::Mat& mat)
{
  std::vector<double> stddevs(mat.cols);

  for (int c = 0; c < mat.cols; ++c)
    stddevs[c] = Stddev<T>(RoiCropRef(mat, c, mat.rows / 2, 1, mat.rows));

  return stddevs;
}

inline cv::Mat LeastSquares(const cv::Mat& Y, const cv::Mat& X)
{
  if (Y.rows != X.rows)
    throw std::runtime_error("Data count mismatch");

  return (X.t() * X).inv() * X.t() * Y;
}

template <typename T>
inline bool Equal(const cv::Mat& mat1, const cv::Mat& mat2, double tolerance = 0.)
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

  for (int r = 0; r < mat1.rows; ++r)
    for (int c = 0; c < mat1.cols; ++c)
      if (static_cast<double>(std::abs(mat1.at<T>(r, c) - mat2.at<T>(r, c))) > tolerance)
        return false;

  return true;
}

template <typename T>
inline T GetQuantile(const std::vector<T>& vec, double quan)
{
  if (vec.empty())
    return 0;
  std::vector<T> out = vec;
  std::sort(out.begin(), out.end());
  return out[(size_t)(quan * (out.size() - 1))];
}

template <typename T>
inline T GetQuantile(cv::Mat&& mat, double quan)
{
  PROFILE_FUNCTION;
  std::span<T> data(reinterpret_cast<T*>(mat.data), mat.total());
  std::ranges::sort(data);
  return data[static_cast<size_t>(std::clamp(quan, 0., 1.) * (data.size() - 1))];
}

template <typename T>
inline cv::Mat QuantileFilter(const cv::Mat& mat, double quantileB, double quantileT)
{
  if (quantileB <= 0 and quantileT >= 1)
    return mat.clone();

  cv::Mat matq = mat.clone();
  matq.convertTo(matq, GetMatType<T>());
  std::vector<T> values(matq.rows * matq.cols);

  for (int r = 0; r < matq.rows; ++r)
    for (int c = 0; c < matq.cols; ++c)
      values[r * matq.cols + c] = matq.at<T>(r, c);

  std::sort(values.begin(), values.end());
  const auto valMin = values[quantileB * (values.size() - 1)];
  const auto valMax = values[quantileT * (values.size() - 1)];

  for (int r = 0; r < matq.rows; ++r)
    for (int c = 0; c < matq.cols; ++c)
      matq.at<T>(r, c) = std::clamp(matq.at<T>(r, c), valMin, valMax);

  return matq;
}
