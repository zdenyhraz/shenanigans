#pragma once

inline cv::Mat roicropref(const cv::Mat& mat, i32 x, i32 y, i32 w, i32 h)
{
  if (x < 0 or y < 0 or x - w / 2 < 0 or y - h / 2 < 0 or x + w / 2 > mat.cols or y + h / 2 > mat.rows)
    [[unlikely]] throw std::runtime_error("roicrop out of bounds");

  return mat(cv::Rect(x - w / 2, y - h / 2, w, h));
}

inline cv::Mat roicrop(const cv::Mat& mat, i32 x, i32 y, i32 w, i32 h)
{
  PROFILE_FUNCTION;
  return roicropref(mat, x, y, w, h).clone();
}

inline cv::Mat roicropmid(const cv::Mat& mat, i32 w, i32 h)
{
  PROFILE_FUNCTION;
  return roicrop(mat, mat.cols / 2, mat.rows / 2, w, h);
}

template <typename T>
inline cv::Mat kirkl(i32 rows, i32 cols, u32 radius)
{
  PROFILE_FUNCTION;
  cv::Mat kirkl = cv::Mat::zeros(rows, cols, std::is_same_v<T, f32> ? CV_32F : CV_64F);
  for (i32 r = 0; r < rows; r++)
    for (i32 c = 0; c < cols; c++)
      kirkl.at<T>(r, c) = 1.0f * ((sqr(r - rows / 2) + sqr(c - cols / 2)) < sqr(radius));

  return kirkl;
}

template <typename T>
inline cv::Mat kirkl(u32 size)
{
  return kirkl<T>(size, size, size / 2);
}

template <typename T>
inline cv::Mat kirklcrop(const cv::Mat& mat, i32 x, i32 y, i32 diameter)
{
  PROFILE_FUNCTION;
  return roicrop(mat, x, y, diameter, diameter).mul(kirkl<T>(diameter));
}
