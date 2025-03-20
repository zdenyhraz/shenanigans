#include <gtest/gtest.h>
#include "Math/Fourier.hpp"
#include "ImageRegistration/IPC.hpp"

TEST(FourierTest, ForwardInverseConsistency)
{
  cv::Mat img(1000, 1000, CV_32F);
  cv::randu(img, cv::Scalar(0), cv::Scalar(1));

  auto fft = FFT(img);
  ASSERT_EQ(fft.size(), img.size());
  ASSERT_EQ(fft.depth(), img.depth());
  ASSERT_EQ(fft.channels(), 2);

  auto ifft = IFFT(fft);
  ASSERT_EQ(ifft.size(), img.size());
  ASSERT_EQ(ifft.depth(), img.depth());
  ASSERT_EQ(ifft.channels(), img.channels());

  for (i32 r = 0; r < img.rows; ++r)
    for (i32 c = 0; c < img.cols; ++c)
      ASSERT_NEAR(ifft.at<f32>(r, c), img.at<f32>(r, c), 1e-6);
}
