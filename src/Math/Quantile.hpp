#pragma once

template <typename T>
inline f64 GetQuantile(const std::vector<T>& vec, f64 quan)
{
  std::vector<T> out = vec;
  std::sort(out.begin(), out.end());
  return out[(usize)(quan * (out.size() - 1))];
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
