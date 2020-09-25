#pragma once
#include "Core/functionsBaseCV.h"

inline Mat vectToMat(std::vector<double> &vec) { return Mat(vec).reshape(0, vec.size()); }

inline std::vector<Mat> vect2ToMats(std::vector<std::vector<double>> &vec)
{
  std::vector<Mat> result(vec.size());
  for (int i = 0; i < vec.size(); i++)
    result[i] = Mat(vec[i]).reshape(0, vec[i].size());
  return result;
}

inline std::vector<double> mat1ToVect(const Mat &mat)
{
  std::vector<double> result(mat.rows, 0);
  for (int r = 0; r < mat.rows; r++)
    result[r] = mat.at<float>(r, 0);
  return result;
}

inline std::vector<std::vector<double>> matToVect2(const Mat &mat)
{
  std::vector<std::vector<double>> result = zerovect2(mat.rows, mat.cols, 0.);
  for (int r = 0; r < mat.rows; r++)
    for (int c = 0; c < mat.cols; c++)
      result[r][c] = mat.at<float>(r, c);
  return result;
}

inline Mat matFromVector(std::vector<double> &vec, int cols)
{
  int rows = vec.size();
  Mat result = Mat::zeros(rows, cols, CV_32F);
  for (int r = 0; r < rows; r++)
  {
    for (int c = 0; c < cols; c++)
    {
      result.at<float>(r, c) = vec[r];
    }
  }
  return result;
}

inline Mat matFromVector(const std::vector<std::vector<double>> &vec, bool transpose = false)
{
  if (transpose)
  {
    int cols = vec.size();
    int rows = vec[0].size();
    Mat result = Mat::zeros(rows, cols, CV_32F);
    for (int r = 0; r < rows; r++)
      for (int c = 0; c < cols; c++)
        result.at<float>(r, c) = vec[c][r];

    return result;
  }
  else
  {
    int rows = vec.size();
    int cols = vec[0].size();
    Mat result = Mat::zeros(rows, cols, CV_32F);
    for (int r = 0; r < rows; r++)
      for (int c = 0; c < cols; c++)
        result.at<float>(r, c) = vec[r][c];

    return result;
  }
}

inline std::vector<double> meanHorizontal(const std::vector<std::vector<double>> &vec)
{
  std::vector<double> meansH(vec.size(), 0);
  Mat mat = matFromVector(vec);

  for (int r = 0; r < mat.rows; r++)
  {
    for (int c = 0; c < mat.cols; c++)
    {
      meansH[r] += mat.at<float>(r, c);
    }
    meansH[r] /= mat.cols;
  }
  return meansH;
}

inline std::vector<double> meanVertical(const std::vector<std::vector<double>> &vec)
{
  std::vector<double> meansV(vec[0].size(), 0);
  Mat mat = matFromVector(vec);

  for (int c = 0; c < mat.cols; c++)
  {
    for (int r = 0; r < mat.rows; r++)
    {
      meansV[c] += mat.at<float>(r, c);
    }
    meansV[c] /= mat.rows;
  }
  return meansV;
}
