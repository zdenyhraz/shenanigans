#pragma once

#include "Core/constants.h"

using namespace Constants;

namespace Procedural
{
inline cv::Mat sinian(int rows, int cols, double rowsFreq, double colsFreq, double rowsShift, double colsShift)
{
  cv::Mat mat = cv::Mat::zeros(rows, cols, CV_32F);
  for (int r = 0; r < rows; r++)
  {
    for (int c = 0; c < cols; c++)
    {
      mat.at<float>(r, c) = sin(colsFreq * TwoPi * ((double)c / (cols - 1) + colsShift)) + sin(rowsFreq * TwoPi * ((double)r / (rows - 1) + rowsShift));
    }
  }
  return mat;
}

inline cv::Mat gaussian(int rows, int cols, double rowsSigma, double colsSigma, double rowsShift, double colsShift)
{
  cv::Mat mat = cv::Mat::zeros(rows, cols, CV_32F);
  for (int r = 0; r < rows; r++)
  {
    for (int c = 0; c < cols; c++)
    {
      mat.at<float>(r, c) = exp(-pow((double)c / (cols - 1) - colsShift, 2) / colsSigma - pow((double)r / (rows - 1) - rowsShift, 2) / rowsSigma);
    }
  }
  return mat;
}
}
