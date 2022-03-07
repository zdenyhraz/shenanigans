#include "Filtering/Median.hpp"

TEST(MedianBlur, OpenCVEquality)
{
  const i32 medsize = 5;
  cv::Mat mat = LoadUnitFloatImage<f32>("../test/data/baboon.png");
  cv::Mat medcv;
  cv::medianBlur(mat, medcv, medsize);
  const auto med = MedianBlur<f32>(mat, medsize, medsize);
  EXPECT_TRUE(Equal<f32>(med, medcv));
}

TEST(MedianBlur, OnePixelKernel)
{
  cv::Mat mat = LoadUnitFloatImage<f32>("../test/data/baboon.png");
  const auto med = MedianBlur<f32>(mat, 1, 1);
  EXPECT_TRUE(Equal<f32>(med, mat));
}
