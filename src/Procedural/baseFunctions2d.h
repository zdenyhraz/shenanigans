#pragma once

#include "Core/constants.h"

using namespace Constants;

namespace Procedural
{
inline cv::Mat sinian(i32 rows, i32 cols, f64 rowsFreq, f64 colsFreq, f64 rowsShift, f64 colsShift)
{
  cv::Mat mat = cv::Mat::zeros(rows, cols, CV_32F);
  for (i32 r = 0; r < rows; r++)
  {
    for (i32 c = 0; c < cols; c++)
    {
      mat.at<f32>(r, c) = sin(colsFreq * TwoPi * ((f64)c / (cols - 1) + colsShift)) + sin(rowsFreq * TwoPi * ((f64)r / (rows - 1) + rowsShift));
    }
  }
  return mat;
}

inline cv::Mat gaussian(i32 rows, i32 cols, f64 rowsSigma, f64 colsSigma, f64 rowsShift, f64 colsShift)
{
  cv::Mat mat = cv::Mat::zeros(rows, cols, CV_32F);
  for (i32 r = 0; r < rows; r++)
  {
    for (i32 c = 0; c < cols; c++)
    {
      mat.at<f32>(r, c) = exp(-pow((f64)c / (cols - 1) - colsShift, 2) / colsSigma - pow((f64)r / (rows - 1) - rowsShift, 2) / rowsSigma);
    }
  }
  return mat;
}
}
