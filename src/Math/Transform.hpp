#pragma once

inline void Shift(cv::Mat& mat, const cv::Point2d& shift)
{
  PROFILE_FUNCTION;
  cv::Mat T = (cv::Mat_<double>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
  cv::warpAffine(mat, mat, T, mat.size());
}

inline cv::Mat Shifted(const cv::Mat& mat, const cv::Point2d& shift)
{
  PROFILE_FUNCTION;
  cv::Mat T = (cv::Mat_<double>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
  cv::Mat shifted;
  cv::warpAffine(mat, shifted, T, mat.size());
  return shifted;
}

inline void Rotate(cv::Mat& mat, double rot, double scale = 1)
{
  PROFILE_FUNCTION;
  cv::Point2d center(mat.cols / 2, mat.rows / 2);
  cv::Mat R = getRotationMatrix2D(center, rot, scale);
  cv::warpAffine(mat, mat, R, mat.size());
}

inline cv::Mat Rotated(const cv::Mat& mat, double rot, double scale = 1)
{
  PROFILE_FUNCTION;
  cv::Point2d center(mat.cols / 2, mat.rows / 2);
  cv::Mat R = getRotationMatrix2D(center, rot, scale);
  cv::Mat rotated;
  cv::warpAffine(mat, rotated, R, mat.size());
  return rotated;
}
