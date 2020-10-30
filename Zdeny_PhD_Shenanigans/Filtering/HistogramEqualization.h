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

std::vector<float> CalculateCummulativeHistogram(const Mat &img)
{
  std::vector<float> hist(256, 0);

  for (int r = 0; r < img.rows; ++r)
    for (int c = 0; c < img.cols; ++c)
      hist[img.at<uchar>(r, c)]++;

  for (int i = 1; i < hist.size(); ++i)
    hist[i] = hist[i - 1] + hist[i];

  return hist;
}

void ShowHistogram(const Mat &img, const std::string &plotname)
{
  auto hist_ = CalculateHistogram(img);
  auto chist_ = CalculateCummulativeHistogram(img);

  auto hist = std::vector<double>(hist_.begin(), hist_.end());
  auto chist = std::vector<double>(chist_.begin(), chist_.end());
  std::vector<double> x(hist.size());
  std::iota(x.begin(), x.end(), 0);

  Plot1D::plot(x, hist, chist, plotname, "pixel value", "histogram", "cummulative histogram");
}

Mat EqualizeHistogram(const Mat &img)
{
  Mat out = img.clone();
  auto cumhist = CalculateCummulativeHistogram(img);

#pragma omp parallel for
  for (int r = 0; r < img.rows; ++r)
    for (int c = 0; c < img.cols; ++c)
      out.at<uchar>(r, c) = static_cast<uchar>(cumhist[img.at<uchar>(r, c)] / cumhist.back() * 255);

  return out;
}
