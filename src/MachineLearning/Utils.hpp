#pragma once

template <typename T>
std::vector<T> ToVector(torch::Tensor x)
{
  std::vector<f32> xvec(x.data_ptr<f32>(), x.data_ptr<f32>() + x.numel());

  if constexpr (std::is_same_v<T, f32>)
    return xvec;
  else
    return std::vector<f64>(xvec.begin(), xvec.end());
}
