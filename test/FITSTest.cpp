#include "Astrophysics/FITS.h"

TEST(FITSTest, LoadFileNotFound)
{
  FitsImage fimg("../resources/FITS/HMI.feets");
  ASSERT_TRUE(fimg.image().empty());
}

TEST(FITSTest, Values)
{
  FitsImage fimg("../resources/FITS/HMI.fits");
  cv::Mat img = cv::imread("../resources/FITS/HMI.png", cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);

  ASSERT_TRUE(not fimg.image().empty());
  ASSERT_TRUE(not img.empty());
  ASSERT_EQ(fimg.image().type(), CV_16U);
  ASSERT_EQ(img.type(), CV_16U);
  ASSERT_EQ(fimg.image().size(), img.size());

  for (i32 r = 0; r < img.rows; ++r)
    for (i32 c = 0; c < img.cols; ++c)
      ASSERT_EQ(fimg.image().at<u16>(r, c), img.at<u16>(r, c));
}