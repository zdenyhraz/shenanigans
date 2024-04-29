#include <gtest/gtest.h>

#include "Math/Fourier.hpp"
#include "ImageRegistration/IPC.hpp"

TEST(FourierTest, ForwardInverseConsistency)
{
  auto img = LoadUnitFloatImage<f32>(GetProjectDirectoryPath("test/data/baboon.png"));
  ASSERT_EQ(img.depth(), CV_32F);
  ASSERT_EQ(img.channels(), 1);

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
