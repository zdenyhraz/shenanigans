#pragma once

inline cv::Mat kirkl(unsigned size)
{
  cv::Mat kirkl = cv::Mat::zeros(size, size, CV_32F);
  for (i32 r = 0; r < size; r++)
    for (i32 c = 0; c < size; c++)
      kirkl.at<f32>(r, c) = 1.0f * ((sqr(r - size / 2) + sqr(c - size / 2)) <= sqr(size / 2));

  return kirkl;
}

inline cv::Mat kirkl(i32 rows, i32 cols, unsigned radius)
{
  cv::Mat kirkl = cv::Mat::zeros(rows, cols, CV_32F);
  for (i32 r = 0; r < rows; r++)
    for (i32 c = 0; c < cols; c++)
      kirkl.at<f32>(r, c) = 1.0f * ((sqr(r - rows / 2) + sqr(c - cols / 2)) < sqr(radius));

  return kirkl;
}

inline cv::Mat kirklcrop(const cv::Mat& sourceimgIn, i32 x, i32 y, i32 diameter)
{
  return roicrop(sourceimgIn, x, y, diameter, diameter).mul(kirkl(diameter));
}
