#include "Filtering/Median.h"

TEST(MedianBlur, OpenCVConsistency)
{
  const i32 n = 51;
  const i32 medsize = 3;
  cv::Mat mat(n, n, CV_32F);
  for (i32 r = 0; r < mat.rows; ++r)
  {
    auto matp = mat.ptr<f32>(r);
    for (i32 c = 0; c < mat.cols; ++c)
      matp[c] = std::sin(0.1 * (r + c));
  }

  for (i32 i = 0; i < 3 * n; ++i)
    mat.at<f32>(rand() % n, rand() % n) = 1.3;

  cv::Mat medcv;
  cv::medianBlur(mat, medcv, medsize);
  const auto med = MedianBlur<f32>(mat, medsize, medsize);

  EXPECT_TRUE(Equal<f32>(med, medcv));
}