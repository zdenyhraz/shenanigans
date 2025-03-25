#pragma once
#include "Math/Functions.hpp"

inline cv::Mat RoiCropRef(const cv::Mat& mat, int x, int y, int w, int h)
{
  if (x < 0 or y < 0 or x - w / 2 < 0 or y - h / 2 < 0 or x + w / 2 > mat.cols or y + h / 2 > mat.rows) [[unlikely]]
    throw std::runtime_error("RoiCrop out of bounds");

  return mat(cv::Rect(x - w / 2, y - h / 2, w, h));
}

inline cv::Mat RoiCrop(const cv::Mat& mat, int x, int y, int w, int h)
{
  return RoiCropRef(mat, x, y, w, h).clone();
}

inline cv::Mat RoiCropMidRef(const cv::Mat& mat, int w, int h)
{
  return RoiCropRef(mat, mat.cols / 2, mat.rows / 2, w, h);
}

inline cv::Mat RoiCropMid(const cv::Mat& mat, int w, int h)
{
  return RoiCropMidRef(mat, w, h).clone();
}

inline cv::Mat RoiCropClipRef(const cv::Mat& mat, int x, int y, int w, int h)
{
  if (x < 0 or y < 0 or x >= mat.cols or y >= mat.rows) [[unlikely]]
    throw std::runtime_error("RoiCropClip out of bounds");

  // top left point inclusive, bot right point exclusive
  const int ulx = x - w / 2;
  const int uly = y - h / 2;
  const int ulxc = std::clamp(ulx, 0, mat.cols - 1);
  const int ulyc = std::clamp(uly, 0, mat.rows - 1);
  const int brxc = std::clamp(ulx + w, 0, mat.cols);
  const int bryc = std::clamp(uly + h, 0, mat.rows);

  return mat(cv::Rect(cv::Point(ulxc, ulyc), cv::Point(brxc, bryc)));
}

inline cv::Mat RoiCropClip(const cv::Mat& mat, int x, int y, int w, int h)
{
  return RoiCropClipRef(mat, x, y, w, h).clone();
}

template <typename T>
inline cv::Mat RoiCropRep(const cv::Mat& mat, int x, int y, int w, int h)
{
  cv::Mat roi(h, w, GetMatType<T>());
  for (int r = 0; r < roi.rows; ++r)
    for (int c = 0; c < roi.cols; ++c)
      roi.at<T>(r, c) = mat.at<T>(std::clamp(y - h / 2 + r, 0, mat.rows - 1), std::clamp(x - w / 2 + c, 0, mat.cols - 1));
  return roi;
}
