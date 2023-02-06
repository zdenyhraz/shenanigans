#include "Math/Fourier.hpp"
#include "ImageRegistration/IPC.hpp"

TEST(FourierTest, ForwardInverseConsistency)
{
  auto img = LoadUnitFloatImage<f32>("../test/data/baboon.png");
  ASSERT_EQ(img.depth(), CV_32F);
  ASSERT_EQ(img.channels(), 1);

  auto FFT = FFT(img);
  ASSERT_EQ(FFT.size(), img.size());
  ASSERT_EQ(FFT.depth(), img.depth());
  ASSERT_EQ(FFT.channels(), 2);

  auto IFFT = IFFT(FFT);
  ASSERT_EQ(IFFT.size(), img.size());
  ASSERT_EQ(IFFT.depth(), img.depth());
  ASSERT_EQ(IFFT.channels(), img.channels());

  for (i32 r = 0; r < img.rows; ++r)
    for (i32 c = 0; c < img.cols; ++c)
      ASSERT_NEAR(IFFT.at<f32>(r, c), img.at<f32>(r, c), 1e-6);
}
