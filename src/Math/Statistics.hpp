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

template <typename T>
inline T GetQuantile(const std::vector<T>& vec, f64 quan)
{
  if (vec.empty())
    return 0;
  std::vector<T> out = vec;
  std::sort(out.begin(), out.end());
  return out[(usize)(quan * (out.size() - 1))];
}

template <typename T>
inline T GetQuantile(const cv::Mat& mat, f64 quan)
{
  std::vector<T> data;
  data.assign(reinterpret_cast<const T*>(mat.datastart), reinterpret_cast<const T*>(mat.dataend));
  std::ranges::sort(data);
  return data[static_cast<usize>(std::clamp(quan, 0., 1.) * (data.size() - 1))];
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
