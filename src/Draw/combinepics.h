#pragma once
#include "Core/functionsBaseCV.h"
#include "Draw/colormap.h"

enum CombineType
{
  COMBINE_JET,
  COMBINE_BIN
};

inline cv::Mat combinePics(const cv::Mat& img1In, const cv::Mat& img2In, CombineType ctype, f64 quanB = 0, f64 quanT = 1)
{
  cv::Mat out = cv::Mat::zeros(img1In.rows, img1In.cols, CV_32FC3);
  cv::Mat img1 = img1In.clone();
  cv::Mat img2 = img2In.clone();
  resize(img2, img2, cv::Size(img1.cols, img1.rows));
  img1.convertTo(img1, CV_32F);
  img2.convertTo(img2, CV_32F);
  normalize(img1, img1, 0, 1, cv::NORM_MINMAX);
  normalize(img2, img2, 0, 1, cv::NORM_MINMAX);
  img1 = applyQuantile(img1, quanB, quanT);
  img2 = applyQuantile(img2, quanB, quanT);

  switch (ctype)
  {
  case COMBINE_JET:
  {
    for (i32 r = 0; r < out.rows; r++)
    {
      for (i32 c = 0; c < out.cols; c++)
      {
        f32 x1 = img1.at<f32>(r, c);
        f32 x2 = img2.at<f32>(r, c);
        cv::Scalar jet = colorMapJet(x2, 0, 1, 1);
        out.at<cv::Vec3f>(r, c)[0] = x1 * jet[0];
        out.at<cv::Vec3f>(r, c)[1] = x1 * jet[1];
        out.at<cv::Vec3f>(r, c)[2] = x1 * jet[2];
      }
    }
    break;
  }
  case COMBINE_BIN:
  {

    break;
  }
  }

  return out;
}
