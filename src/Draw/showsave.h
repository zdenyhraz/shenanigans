#pragma once
#include "Core/functionsBaseCV.h"
#include "Draw/colormap.h"

inline void showimg(const cv::Mat& sourceimgIn, std::string windowname, bool color = false, double quantileB = 0, double quantileT = 1, int wRows = 600)
{
  cv::Mat sourceimg = sourceimgIn.clone();

  double colRowRatio = (double)sourceimg.cols / (double)sourceimg.rows;
  int wCols = (double)wRows * colRowRatio;
  cv::namedWindow(windowname, cv::WINDOW_NORMAL);
  cv::resizeWindow(windowname, wCols, wRows);

  sourceimg.convertTo(sourceimg, CV_32F);
  normalize(sourceimg, sourceimg, 0, 1, cv::NORM_MINMAX);

  if (sourceimg.channels() > 1 && (quantileB != 0 || quantileT != 1))
    LOG_WARNING("Quantile clipping not implemented for color images");

  if (sourceimg.channels() == 1)
  {
    if (color)
      sourceimg = applyQuantileColorMap(sourceimg, quantileB, quantileT);
    else if (quantileB != 0 || quantileT != 1)
      sourceimg = applyQuantile(sourceimg, quantileB, quantileT);
  }

  imshow(windowname, sourceimg);
  cv::waitKey(1);
}

inline void showimg(const std::vector<cv::Mat>& sourceimgIns, std::string windowname, bool color = false, double quantileB = 0, double quantileT = 1, int wRows = 600)
{
  // 1st image determines the main hconcat height
  int mainHeight = sourceimgIns[0].rows;
  std::vector<cv::Mat> sourceimgs;
  sourceimgs.reserve(sourceimgIns.size());
  for (auto& srcimg : sourceimgIns)
  {
    if (!srcimg.empty())
    {
      auto img = srcimg.clone();
      resize(img, img, cv::Size((double)mainHeight / img.rows * img.cols, mainHeight));
      normalize(img, img, 0, 255, cv::NORM_MINMAX);
      img.convertTo(img, CV_8U);
      sourceimgs.emplace_back(img);
    }
  }

  cv::Mat concatenated;
  hconcat(sourceimgs, concatenated);
  showimg(concatenated, windowname, color, quantileB, quantileT, wRows);
}

inline void saveimg(std::string path, const cv::Mat& sourceimgIn, bool bilinear = false, cv::Size size = cv::Size(0, 0), bool color = false, double quantileB = 0, double quantileT = 1)
{
  cv::Mat saveimg = sourceimgIn.clone();

  if (size != cv::Size2i(0, 0))
  {
    if (bilinear)
      resize(sourceimgIn, saveimg, size, 0, 0, cv::INTER_LINEAR);
    else
      resize(sourceimgIn, saveimg, size, 0, 0, cv::INTER_NEAREST);
  }

  normalize(saveimg, saveimg, 0, 255, cv::NORM_MINMAX);
  saveimg.convertTo(saveimg, CV_8U);

  if (saveimg.channels() == 1)
  {
    if (color)
      saveimg = applyQuantileColorMap(saveimg, quantileB, quantileT);
    else if (quantileB != 0 || quantileT != 1)
      saveimg = applyQuantile(saveimg, quantileB, quantileT);
  }

  imwrite(path, saveimg);
  LOG_DEBUG("Saved image to {}", path);
}
