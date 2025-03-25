#pragma once

template <typename T>
inline std::vector<double> GetRow(const cv::Mat& mat, int row)
{
  std::vector<double> out(mat.cols);
  for (int c = 0; c < mat.cols; ++c)
    out[c] = mat.at<T>(row, c);
  return out;
}

template <typename T>
inline std::vector<double> GetCol(const cv::Mat& mat, int col)
{
  std::vector<double> out(mat.rows);
  for (int r = 0; r < mat.rows; ++r)
    out[r] = mat.at<T>(r, col);
  return out;
}

template <typename T>
inline std::vector<double> GetMidRow(const cv::Mat& img)
{
  std::vector<double> out(img.cols);
  for (int c = 0; c < img.cols; ++c)
    out[c] = img.at<T>(img.rows / 2, c);
  return out;
}

template <typename T>
inline std::vector<double> GetMidCol(const cv::Mat& img)
{
  std::vector<double> out(img.rows);
  for (int r = 0; r < img.rows; ++r)
    out[r] = img.at<T>(r, img.cols / 2);
  return out;
}

template <typename T>
inline std::vector<double> GetDiagonal(const cv::Mat& img)
{
  std::vector<double> out(std::min(img.rows, img.cols));
  for (int c = 0; c < std::min(img.rows, img.cols); ++c)
    out[c] = img.at<T>(c, c);
  return out;
}
