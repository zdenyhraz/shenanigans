#pragma once
#include "UtilsCV/FunctionsBaseCV.h"

inline cv::Mat vectToMat(std::vector<f64>& vec)
{
  return cv::Mat(vec).reshape(0, vec.size());
}

inline std::vector<cv::Mat> vect2ToMats(std::vector<std::vector<f64>>& vec)
{
  std::vector<cv::Mat> result(vec.size());
  for (usize i = 0; i < vec.size(); i++)
    result[i] = cv::Mat(vec[i]).reshape(0, vec[i].size());
  return result;
}

inline std::vector<f64> mat1ToVect(const cv::Mat& mat)
{
  std::vector<f64> result(mat.rows, 0);
  for (i32 r = 0; r < mat.rows; r++)
    result[r] = mat.at<f32>(r, 0);
  return result;
}

inline std::vector<std::vector<f64>> matToVect2(const cv::Mat& mat)
{
  std::vector<std::vector<f64>> result = zerovect2(mat.rows, mat.cols, 0.);
  for (i32 r = 0; r < mat.rows; r++)
    for (i32 c = 0; c < mat.cols; c++)
      result[r][c] = mat.at<f32>(r, c);
  return result;
}

inline cv::Mat matFromVector(std::vector<f64>& vec, i32 cols)
{
  i32 rows = vec.size();
  cv::Mat result = cv::Mat::zeros(rows, cols, CV_32F);
  for (i32 r = 0; r < rows; r++)
  {
    for (i32 c = 0; c < cols; c++)
    {
      result.at<f32>(r, c) = vec[r];
    }
  }
  return result;
}

inline cv::Mat matFromVector(const std::vector<std::vector<f64>>& vec, bool transpose = false)
{
  if (transpose)
  {
    i32 cols = vec.size();
    i32 rows = vec[0].size();
    cv::Mat result = cv::Mat::zeros(rows, cols, CV_32F);
    for (i32 r = 0; r < rows; r++)
      for (i32 c = 0; c < cols; c++)
        result.at<f32>(r, c) = vec[c][r];

    return result;
  }
  else
  {
    i32 rows = vec.size();
    i32 cols = vec[0].size();
    cv::Mat result = cv::Mat::zeros(rows, cols, CV_32F);
    for (i32 r = 0; r < rows; r++)
      for (i32 c = 0; c < cols; c++)
        result.at<f32>(r, c) = vec[r][c];

    return result;
  }
}

inline std::vector<f64> meanHorizontal(const std::vector<std::vector<f64>>& vec)
{
  std::vector<f64> meansH(vec.size(), 0);
  cv::Mat mat = matFromVector(vec);

  for (i32 r = 0; r < mat.rows; r++)
  {
    for (i32 c = 0; c < mat.cols; c++)
    {
      meansH[r] += mat.at<f32>(r, c);
    }
    meansH[r] /= mat.cols;
  }
  return meansH;
}

inline std::vector<f64> meanVertical(const std::vector<std::vector<f64>>& vec)
{
  std::vector<f64> meansV(vec[0].size(), 0);
  cv::Mat mat = matFromVector(vec);

  for (i32 c = 0; c < mat.cols; c++)
  {
    for (i32 r = 0; r < mat.rows; r++)
    {
      meansV[c] += mat.at<f32>(r, c);
    }
    meansV[c] /= mat.rows;
  }
  return meansV;
}

inline std::vector<f64> GetRow(const cv::Mat& mat, i32 row)
{
  std::vector<f64> out(mat.cols);
  for (i32 c = 0; c < mat.cols; ++c)
    out[c] = mat.at<f64>(row, c);
  return out;
}

inline std::vector<f64> GetCol(const cv::Mat& mat, i32 col)
{
  std::vector<f64> out(mat.rows);
  for (i32 r = 0; r < mat.rows; ++r)
    out[r] = mat.at<f64>(r, col);
  return out;
}
