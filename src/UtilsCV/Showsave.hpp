#pragma once
#include "Colormap.hpp"

inline void Showimg(const cv::Mat& sourceimgIn, const std::string& windowname, bool color = false, f64 quantileB = 0, f64 quantileT = 1, i32 wRows = 600)
{
  cv::Mat sourceimg = sourceimgIn.clone();

  i32 wCols = static_cast<f32>(sourceimg.cols) * wRows / sourceimg.rows;
  cv::namedWindow(windowname, cv::WINDOW_NORMAL);
  cv::resizeWindow(windowname, wCols, wRows);

  sourceimg.convertTo(sourceimg, CV_32F);
  cv::normalize(sourceimg, sourceimg, 0, 1, cv::NORM_MINMAX);

  if (sourceimg.channels() > 1 and (quantileB != 0 or quantileT != 1))
    LOG_WARNING("Quantile clipping not implemented for color images");

  if (sourceimg.channels() == 1)
  {
    if (quantileB != 0 or quantileT != 1)
      sourceimg = QuantileFilter<f32>(sourceimg, quantileB, quantileT);

    if (color)
    {
      cv::Mat cmap;
      cv::normalize(sourceimg, sourceimg, 0, 255, cv::NORM_MINMAX);
      sourceimg.convertTo(sourceimg, CV_8U);
      cv::applyColorMap(sourceimg, cmap, cv::COLORMAP_JET);
      sourceimg = cmap;
    }
  }

  cv::imshow(windowname, sourceimg);
  cv::waitKey(1);
}

inline void Showimg(const std::vector<cv::Mat>& sourceimgIns, const std::string& windowname, bool color = false, f64 quantileB = 0, f64 quantileT = 1, i32 wRows = 600)
{
  i32 mainHeight = sourceimgIns[0].rows;
  std::vector<cv::Mat> sourceimgs;
  sourceimgs.reserve(sourceimgIns.size());
  for (auto& srcimg : sourceimgIns)
  {
    if (!srcimg.empty())
    {
      auto img = srcimg.clone();
      cv::resize(img, img, cv::Size((f64)mainHeight / img.rows * img.cols, mainHeight));
      cv::normalize(img, img, 0, 255, cv::NORM_MINMAX);
      img.convertTo(img, CV_8U);
      sourceimgs.emplace_back(img);
    }
  }

  cv::Mat concatenated;
  hconcat(sourceimgs, concatenated);
  Showimg(concatenated, windowname, color, quantileB, quantileT, wRows);
}

inline void Saveimg(
    const std::string& path, const cv::Mat& sourceimgIn, bool bilinear = false, cv::Size size = cv::Size(0, 0), bool color = false, f64 quantileB = 0, f64 quantileT = 1)
{
  cv::Mat img = sourceimgIn.clone();

  if (size != cv::Size2i(0, 0))
  {
    if (bilinear)
      cv::resize(sourceimgIn, img, size, 0, 0, cv::INTER_LINEAR);
    else
      cv::resize(sourceimgIn, img, size, 0, 0, cv::INTER_NEAREST);
  }

  cv::normalize(img, img, 0, 255, cv::NORM_MINMAX);
  img.convertTo(img, CV_8U);

  if (img.channels() == 1)
  {
    if (quantileB != 0 or quantileT != 1)
      img = QuantileFilter<f32>(img, quantileB, quantileT);

    if (color)
    {
      cv::Mat cmap;
      cv::applyColorMap(img, cmap, cv::COLORMAP_JET);
      img = cmap;
    }
  }

  cv::imwrite(path, img);
  LOG_DEBUG("Saved image to {}", std::filesystem::weakly_canonical(path).string());
}
