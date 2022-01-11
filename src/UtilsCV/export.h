#pragma once
#include "UtilsCV/FunctionsBaseCV.h"

inline void saveMatToCsv(const std::string& path, const cv::Mat& matIn)
{
  std::ofstream listing(path, std::ios::out | std::ios::trunc);
  cv::Mat mat = matIn.clone();
  mat.convertTo(mat, CV_32F);
  for (i32 r = 0; r < mat.rows; r++)
  {
    for (i32 c = 0; c < mat.cols - 1; c++)
    {
      listing << mat.at<f32>(r, c) << ",";
    }
    listing << mat.at<f32>(r, mat.cols - 1) << std::endl;
  }
}

inline void exportToMATLAB(const cv::Mat& Zdata, f64 xmin, f64 xmax, f64 ymin, f64 ymax, std::string path)
{
  std::ofstream listing(path, std::ios::out | std::ios::trunc);
  listing << xmin << "," << xmax << "," << ymin << "," << ymax << std::endl;
  for (i32 r = 0; r < Zdata.rows; r++)
    for (i32 c = 0; c < Zdata.cols; c++)
      listing << Zdata.at<f32>(r, c) << std::endl;
}