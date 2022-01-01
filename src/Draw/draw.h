#pragma once
#include "Core/functionsBaseCV.h"

inline cv::Mat crosshair(const cv::Mat& sourceimgIn, cv::Point point)
{
  cv::Mat sourceimg = sourceimgIn.clone();
  cv::Scalar CrosshairColor = cv::Scalar(0);
  i32 thickness = std::max(sourceimg.cols / 150, 1);
  i32 inner = sourceimg.cols / 50;
  i32 outer = sourceimg.cols / 18;
  cv::Point offset1(0, outer);
  cv::Point offset2(outer, 0);
  cv::Point offset3(0, inner);
  cv::Point offset4(inner, 0);
  normalize(sourceimg, sourceimg, 0, 255, cv::NORM_MINMAX);
  sourceimg.convertTo(sourceimg, CV_8U);

  circle(sourceimg, point, inner, CrosshairColor, thickness, 0, 0);
  line(sourceimg, point + offset3, point + offset1, CrosshairColor, thickness);
  line(sourceimg, point + offset4, point + offset2, CrosshairColor, thickness);
  line(sourceimg, point - offset3, point - offset1, CrosshairColor, thickness);
  line(sourceimg, point - offset4, point - offset2, CrosshairColor, thickness);
  return sourceimg;
}

inline cv::Mat xko(const cv::Mat& sourceimgIn, cv::Point point, cv::Scalar CrosshairColor, std::string inputType)
{
  cv::Mat sourceimg = sourceimgIn.clone();
  i32 inner = 0;
  i32 outer = 4;
  cv::Point offset1(-outer, -outer);
  cv::Point offset2(outer, -outer);
  cv::Point offset3(outer, outer);
  cv::Point offset4(-outer, outer);
  cv::Mat sourceimgCLR = cv::Mat::zeros(sourceimg.size(), CV_16UC3);
  if (inputType == "CLR")
  {
    normalize(sourceimg, sourceimg, 0, 65535, cv::NORM_MINMAX);
    sourceimg.convertTo(sourceimg, CV_16UC3);
    sourceimg.copyTo(sourceimgCLR);
  }
  if (inputType == "GRS")
  {
    normalize(sourceimg, sourceimg, 0, 65535, cv::NORM_MINMAX);
    sourceimg.convertTo(sourceimg, CV_16UC1);
    for (i32 r = 0; r < sourceimg.rows; r++)
    {
      for (i32 c = 0; c < sourceimg.cols; c++)
      {
        sourceimgCLR.at<cv::Vec3w>(r, c)[0] = sourceimg.at<ushort>(r, c);
        sourceimgCLR.at<cv::Vec3w>(r, c)[1] = sourceimg.at<ushort>(r, c);
        sourceimgCLR.at<cv::Vec3w>(r, c)[2] = sourceimg.at<ushort>(r, c);
      }
    }
  }
  f64 THICCness = 1;
  // circle(sourceimgCLR, point, inner, CrosshairColor, 2);
  circle(sourceimgCLR, point, 5, CrosshairColor, THICCness, 0, 0);
  line(sourceimgCLR, point + offset3, point + offset1, CrosshairColor, THICCness);
  line(sourceimgCLR, point + offset4, point + offset2, CrosshairColor, THICCness);
  line(sourceimgCLR, point - offset3, point - offset1, CrosshairColor, THICCness);
  line(sourceimgCLR, point - offset4, point - offset2, CrosshairColor, THICCness);
  return sourceimgCLR;
}

inline cv::Mat pointik(const cv::Mat& sourceimgIn, cv::Point point, cv::Scalar CrosshairColor, std::string inputType)
{
  cv::Mat sourceimg = sourceimgIn.clone();
  i32 inner = 1;
  i32 outer = 0;
  cv::Point offset1(-outer, -outer);
  cv::Point offset2(outer, -outer);
  cv::Point offset3(outer, outer);
  cv::Point offset4(-outer, outer);
  cv::Mat sourceimgCLR = cv::Mat::zeros(sourceimg.size(), CV_16UC3);
  if (inputType == "CLR")
  {
    normalize(sourceimg, sourceimg, 0, 65535, cv::NORM_MINMAX);
    sourceimg.convertTo(sourceimg, CV_16UC1);
    sourceimg.copyTo(sourceimgCLR);
  }
  if (inputType == "GRS")
  {
    normalize(sourceimg, sourceimg, 0, 65535, cv::NORM_MINMAX);
    sourceimg.convertTo(sourceimg, CV_16UC1);
    for (i32 r = 0; r < sourceimg.rows; r++)
    {
      for (i32 c = 0; c < sourceimg.cols; c++)
      {
        sourceimgCLR.at<cv::Vec3w>(r, c)[0] = sourceimg.at<ushort>(r, c);
        sourceimgCLR.at<cv::Vec3w>(r, c)[1] = sourceimg.at<ushort>(r, c);
        sourceimgCLR.at<cv::Vec3w>(r, c)[2] = sourceimg.at<ushort>(r, c);
      }
    }
  }
  circle(sourceimgCLR, point, inner, CrosshairColor, 2);
  return sourceimgCLR;
}

inline void drawPoint(cv::Mat& out, const cv::Point& pt, const cv::Scalar& clr, i32 ptsz = 5, i32 thickness = 1)
{
  cv::Point ofst1(ptsz, ptsz);
  cv::Point ofst2(ptsz, -ptsz);
  line(out, pt - ofst1, pt + ofst1, clr, thickness);
  line(out, pt - ofst2, pt + ofst2, clr, thickness);
}