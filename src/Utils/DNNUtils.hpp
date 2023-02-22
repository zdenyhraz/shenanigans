#pragma once
#include "Formatters.hpp"

template <typename T = f32>
inline std::vector<T> ToStdVector(torch::Tensor x)
{
  std::vector<f32> xvec(x.data_ptr<f32>(), x.data_ptr<f32>() + x.numel());

  if constexpr (std::is_same_v<T, f32>)
    return xvec;
  else
    return std::vector<T>(xvec.begin(), xvec.end());
}

inline cv::Mat ToCVMat(torch::Tensor x, cv::Size size)
{
  return cv::Mat(size, CV_32F, x.data_ptr()).clone();
}

inline torch::Tensor ToTensor(const cv::Mat& image)
{
  if (image.channels() == 1)
    return torch::from_blob(image.data, {image.rows * image.cols});

  return torch::from_blob(image.data, {image.channels(), image.rows * image.cols});
}
