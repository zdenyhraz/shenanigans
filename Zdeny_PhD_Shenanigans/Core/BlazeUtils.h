#pragma once
#include "stdafx.h"

using BlazeMat = blaze::DynamicMatrix<float>;

inline BlazeMat LoadImageBlaze(const std::string& path)
{
  Mat img = loadImage(path);

  BlazeMat out(img.rows, img.cols);
  for (int r = 0; r < out.rows(); ++r)
    for (int c = 0; c < out.columns(); ++c)
      out(r, c) = img.at<float>(r, c);

  return out;
}
