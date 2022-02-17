#pragma once

inline cv::Mat roicropref(const cv::Mat& mat, i32 x, i32 y, i32 w, i32 h)
{
  if (x < 0 or y < 0 or x - w / 2 < 0 or y - h / 2 < 0 or x + w / 2 > mat.cols or y + h / 2 > mat.rows)
    [[unlikely]] throw std::runtime_error("roicrop out of bounds");

  return mat(cv::Rect(x - w / 2, y - h / 2, w, h));
}

inline cv::Mat roicrop(const cv::Mat& mat, i32 x, i32 y, i32 w, i32 h)
{
  return roicropref(mat, x, y, w, h).clone();
}

inline cv::Mat roicropmid(const cv::Mat& mat, i32 w, i32 h)
{
  return roicrop(mat, mat.cols / 2, mat.rows / 2, w, h);
}

inline cv::Mat kirkl(i32 rows, i32 cols, u32 radius)
{
  cv::Mat kirkl = cv::Mat::zeros(rows, cols, CV_32F);
  for (i32 r = 0; r < rows; r++)
    for (i32 c = 0; c < cols; c++)
      kirkl.at<f32>(r, c) = 1.0f * ((sqr(r - rows / 2) + sqr(c - cols / 2)) < sqr(radius));

  return kirkl;
}

inline cv::Mat kirkl(u32 size)
{
  return kirkl(size, size, size / 2);
}

inline cv::Mat kirklcrop(const cv::Mat& mat, i32 x, i32 y, i32 diameter)
{
  return roicrop(mat, x, y, diameter, diameter).mul(kirkl(diameter));
}
