#include <gtest/gtest.h>

#include "Math/Statistics.hpp"

TEST(ColMathTest, ColMeansOdd)
{
  cv::Mat mat(3, 2, CV_64F);
  mat.at<f64>(0, 0) = 1;
  mat.at<f64>(1, 0) = 3;
  mat.at<f64>(2, 0) = 5;

  mat.at<f64>(0, 1) = 4;
  mat.at<f64>(1, 1) = 6;
  mat.at<f64>(2, 1) = 8;

  const auto colmeans = ColMeans<f64>(mat);
  ASSERT_EQ(colmeans.size(), 2);
  ASSERT_EQ(colmeans[0], 3);
  ASSERT_EQ(colmeans[1], 6);
}

TEST(ColMathTest, ColMeansEven)
{
  cv::Mat mat(4, 2, CV_64F);
  mat.at<f64>(0, 0) = 1;
  mat.at<f64>(1, 0) = 3;
  mat.at<f64>(2, 0) = 5;
  mat.at<f64>(3, 0) = 7;

  mat.at<f64>(0, 1) = 4;
  mat.at<f64>(1, 1) = 6;
  mat.at<f64>(2, 1) = 8;
  mat.at<f64>(3, 1) = 10;

  const auto colmeans = ColMeans<f64>(mat);
  ASSERT_EQ(colmeans.size(), 2);
  ASSERT_EQ(colmeans[0], 4);
  ASSERT_EQ(colmeans[1], 7);
}
