#include "Fourier/fourier.h"
#include "IPC/IterativePhaseCorrelation.h"

TEST(FourierTest, ForwardBackwardConsistencyTest)
{
  auto img = loadImage("../resources/Shapes/shape.png");
  EXPECT_EQ(img.depth(), CV_32F);
  EXPECT_EQ(img.channels(), 1);

  auto fft = Fourier::fft(img);
  EXPECT_EQ(fft.size(), img.size());
  EXPECT_EQ(fft.depth(), img.depth());
  EXPECT_EQ(fft.channels(), 2);

  auto ifft = Fourier::ifft(fft);
  EXPECT_EQ(ifft.size(), img.size());
  EXPECT_EQ(ifft.depth(), img.depth());
  EXPECT_EQ(ifft.channels(), img.channels());

  for (i32 r = 0; r < img.rows; ++r)
    for (i32 c = 0; c < img.cols; ++c)
      ASSERT_NEAR(ifft.at<f32>(r, c), img.at<f32>(r, c), 1e-6);
}
