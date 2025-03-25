#include <gtest/gtest.h>
#include "ImageProcessing/MedianBlur.hpp"

TEST(MedianBlur, OpenCVEquality)
{
  const int medsize = 5;
  cv::Mat mat(1000, 1000, CV_32F);
  cv::randu(mat, cv::Scalar(0), cv::Scalar(1));
  cv::Mat medcv;
  cv::medianBlur(mat, medcv, medsize);
  const auto med = MedianBlur<float>(mat, medsize, medsize);
  EXPECT_TRUE(Equal<float>(med, medcv));
}

TEST(MedianBlur, OnePixelKernel)
{
  cv::Mat mat(1000, 1000, CV_32F);
  cv::randu(mat, cv::Scalar(0), cv::Scalar(1));
  const auto med = MedianBlur<float>(mat, 1, 1);
  EXPECT_TRUE(Equal<float>(med, mat));
}
