#pragma once

inline cv::Mat HistogramEqualize(const cv::Mat& img)
{
  cv::Mat sourceimg = img.clone();
  cv::normalize(sourceimg, sourceimg, 0, 255, cv::NORM_MINMAX);
  sourceimg.convertTo(sourceimg, CV_8U);
  equalizeHist(sourceimg, sourceimg);
  sourceimg.convertTo(sourceimg, CV_16U);
  cv::normalize(sourceimg, sourceimg, 0, 65535, cv::NORM_MINMAX);
  std::cout << "histogram equalized" << std::endl;
  return sourceimg;
}

inline std::vector<float> CalculateHistogram(const cv::Mat& img)
{
  std::vector<float> hist(256, 0);
  for (int r = 0; r < img.rows; ++r)
    for (int c = 0; c < img.cols; ++c)
      hist[img.at<uchar>(r, c)]++;

  return hist;
}

inline std::vector<float> CalculateCummulativeHistogram(const cv::Mat& img)
{
  std::vector<float> hist(256, 0);

  for (int r = 0; r < img.rows; ++r)
    for (int c = 0; c < img.cols; ++c)
      hist[img.at<uchar>(r, c)]++;

  for (size_t i = 1; i < hist.size(); ++i)
    hist[i] = hist[i - 1] + hist[i];

  return hist;
}

inline void ShowHistogram(const cv::Mat& img, const std::string& plotname)
{
  auto hist_ = CalculateHistogram(img);
  auto chist_ = CalculateCummulativeHistogram(img);

  auto hist = std::vector<double>(hist_.begin(), hist_.end());
  auto chist = std::vector<double>(chist_.begin(), chist_.end());
  std::vector<double> x(hist.size());
  std::iota(x.begin(), x.end(), 0);

  // Plot1D::Plot(x, hist, chist, plotname, "pixel value", "histogram", "cummulative histogram");
}

inline cv::Mat EqualizeHistogram(const cv::Mat& img)
{
  cv::Mat out = img.clone();
  auto chist = CalculateCummulativeHistogram(img);

  for (int r = 0; r < img.rows; ++r)
    for (int c = 0; c < img.cols; ++c)
      out.at<uchar>(r, c) = static_cast<uchar>(chist[img.at<uchar>(r, c)] / chist.back() * 255);

  return out;
}

inline cv::Mat EqualizeHistogramAdaptive(const cv::Mat& img, int wsize)
{
  cv::Mat out = img.clone();

  if (wsize > img.cols / 2)
  {
    LOG_ERROR("Window size too large for AHEQ");
    return out;
  }

  for (int r = 0; r < img.rows; ++r)
  {
    if (r % 5 == 0)
      LOG_DEBUG("AHEQ progress {:.1f}%", (float)r / (img.rows - 1) * 100);

#pragma omp parallel for
    for (int c = 0; c < img.cols; ++c)
    {
      if (r > wsize / 2 and c > wsize / 2 and r < img.rows - wsize / 2 and c < img.cols - wsize / 2)
      {
        auto chist = CalculateCummulativeHistogram(RoiCrop(img, r, c, wsize, wsize));
        out.at<uchar>(r, c) = static_cast<uchar>(chist[img.at<uchar>(r, c)] / chist.back() * 255);
      }
    }
  }

  return out;
}
