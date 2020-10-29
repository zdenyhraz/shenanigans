#pragma once
#include "stdafx.h"

std::vector<size_t> CalculateHistogram(const Mat &img)
{
  std::vector<size_t> histogram(256, 0);
  for (int r = 0; r < img.rows; ++r)
    for (int c = 0; c < img.cols; ++c)
      histogram[img.at<uchar>(r, c)]++;

  return histogram;
}

std::vector<double> CalculateCummulativeHistogram(const std::vector<size_t> &histogram)
{
  std::vector<double> cumhistogram(256, 0);
  double cumval = 0;
  for (int i = 0; i < histogram.size(); ++i)
  {
    cumval += histogram[i];
    cumhistogram[i] = cumval;
  }

  for (auto &x : cumhistogram)
    x /= cumval;

  return cumhistogram;
}

void ShowHistogram(const Mat &img, const std::string &plotname)
{
  auto hist_ = CalculateHistogram(img);
  auto chist = CalculateCummulativeHistogram(hist_);

  auto hist = std::vector<double>(hist_.begin(), hist_.end());
  std::vector<double> x(hist.size());
  std::iota(x.begin(), x.end(), 0);

  Plot1D::plot(x, hist, chist, plotname, "pixel value", "histogram", "cummulative histogram");
}

Mat EqualizeHistogram(const Mat &img)
{
  Mat out = img.clone();
  auto histogram = CalculateHistogram(img);
  auto cumhistogram = CalculateCummulativeHistogram(histogram);

  for (int r = 0; r < img.rows; ++r)
    for (int c = 0; c < img.cols; ++c)
      out.at<uchar>(r, c) = cumhistogram[img.at<uchar>(r, c)] * 255;

  return out;
}
