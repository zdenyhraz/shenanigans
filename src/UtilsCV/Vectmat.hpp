#pragma once

template <typename T>
inline std::vector<f64> GetRow(const cv::Mat& mat, i32 row)
{
  std::vector<f64> out(mat.cols);
  for (i32 c = 0; c < mat.cols; ++c)
    out[c] = mat.at<T>(row, c);
  return out;
}

template <typename T>
inline std::vector<f64> GetCol(const cv::Mat& mat, i32 col)
{
  std::vector<f64> out(mat.rows);
  for (i32 r = 0; r < mat.rows; ++r)
    out[r] = mat.at<T>(r, col);
  return out;
}

template <typename T>
inline std::vector<f64> GetMidRow(const cv::Mat& img)
{
  std::vector<f64> out(img.cols);
  for (i32 c = 0; c < img.cols; ++c)
    out[c] = img.at<T>(img.rows / 2, c);
  return out;
}

template <typename T>
inline std::vector<f64> GetMidCol(const cv::Mat& img)
{
  std::vector<f64> out(img.rows);
  for (i32 r = 0; r < img.rows; ++r)
    out[r] = img.at<T>(r, img.cols / 2);
  return out;
}

template <typename T>
inline std::vector<f64> GetDiagonal(const cv::Mat& img)
{
  std::vector<f64> out(std::min(img.rows, img.cols));
  for (i32 c = 0; c < std::min(img.rows, img.cols); ++c)
    out[c] = img.at<T>(c, c);
  return out;
}