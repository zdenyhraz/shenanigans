#pragma once
#include "Formatters.hpp"

template <typename T>
std::vector<T> ToStdVector(torch::Tensor x)
{
  std::vector<f32> xvec(x.data_ptr<f32>(), x.data_ptr<f32>() + x.numel());

  if constexpr (std::is_same_v<T, f32>)
    return xvec;
  else
    return std::vector<f64>(xvec.begin(), xvec.end());
}

cv::Mat ToCVMat(torch::Tensor x, cv::Size size)
{
  return cv::Mat(size, CV_32F, x.data_ptr()).clone();
}

torch::Tensor ToTensor(const cv::Mat& image)
{
  return torch::from_blob(image.data, {image.rows * image.cols});
}
