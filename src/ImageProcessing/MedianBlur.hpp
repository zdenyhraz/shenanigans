#pragma once
#include "Math/Statistics.hpp"
#include "Math/Functions.hpp"

template <typename T>
inline T Median(const cv::Mat& mat, std::vector<T>& values)
{
  for (int r = 0; r < mat.rows; ++r)
  {
    auto matp = mat.ptr<T>(r);
    for (int c = 0; c < mat.cols; ++c)
      values[r * mat.cols + c] = matp[c];
  }
  return Median(values);
}

template <typename T>
inline T Median(const cv::Mat& mat)
{
  std::vector<T> values(mat.rows * mat.cols);
  return Median(mat, values);
}

template <typename T>
inline cv::Mat MedianBlur(const cv::Mat& mat, int kwidth, int kheight)
{
  PROFILE_FUNCTION;
  if (kwidth % 2 == 0 or kheight % 2 == 0) [[unlikely]]
    throw std::invalid_argument("Invalid median blur window size");

  cv::Mat med = cv::Mat(mat.size(), GetMatType<T>());
  std::vector<T> values(kwidth * kheight); // buffer for median calculation to prevent re-allocations

  for (int r = 0; r < mat.rows; ++r)
  {
    auto medp = med.ptr<T>(r);
    for (int c = 0; c < mat.cols; ++c)
      medp[c] = Median<T>(RoiCropRep<T>(mat, c, r, kwidth, kheight), values);
  }
  return med;
}
