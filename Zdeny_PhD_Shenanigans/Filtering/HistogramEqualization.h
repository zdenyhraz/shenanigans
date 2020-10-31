#pragma once
#include "stdafx.h"

inline std::vector<float> CalculateHistogram(const Mat &img)
{
  std::vector<float> hist(256, 0);
  for (int r = 0; r < img.rows; ++r)
    for (int c = 0; c < img.cols; ++c)
      hist[img.at<uchar>(r, c)]++;

  return hist;
}

inline std::vector<float> CalculateCummulativeHistogram(const Mat &img)
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
  auto chist = CalculateCummulativeHistogram(img);

  for (int r = 0; r < img.rows; ++r)
    for (int c = 0; c < img.cols; ++c)
      out.at<uchar>(r, c) = static_cast<uchar>(chist[img.at<uchar>(r, c)] / chist.back() * 255);

  return out;
}

Mat EqualizeHistogramAdaptive(const Mat &img, int wsize)
{
  Mat out = img.clone();
  for (int r = 0; r < img.rows; ++r)
  {
    if (r % 5 == 0)
      LOG_DEBUG("AHEQ progress {:.1f}%", (float)r / (img.rows - 1) * 100);

#pragma omp parallel for
    for (int c = 0; c < img.cols; ++c)
    {
      if (r > wsize / 2 && c > wsize / 2 && r < img.rows - wsize / 2 && c < img.cols - wsize / 2)
      {
        auto chist = CalculateCummulativeHistogram(roicrop(img, r, c, wsize, wsize));
        out.at<uchar>(r, c) = static_cast<uchar>(chist[img.at<uchar>(r, c)] / chist.back() * 255);
      }
    }
  }

  return out;
}
