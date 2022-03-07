#pragma once

inline void DrawCrosshairs(cv::Mat& mat)
{
  cv::line(mat, cv::Point(mat.cols / 2, 0), cv::Point(mat.cols / 2, mat.rows - 1), cv::Scalar(0.0f), std::max(mat.cols / 100, 1), cv::LINE_AA);
  cv::line(mat, cv::Point(0, mat.rows / 2), cv::Point(mat.cols - 1, mat.rows / 2), cv::Scalar(0.0f), std::max(mat.cols / 100, 1), cv::LINE_AA);
}

inline void DrawCross(cv::Mat& mat, const cv::Point& point)
{
  cv::line(mat, cv::Point(point.x - mat.cols / 30, point.y - mat.cols / 30), cv::Point(point.x + mat.cols / 30, point.y + mat.cols / 30), cv::Scalar(0.0f), std::max(mat.cols / 100, 1), cv::LINE_AA);
  cv::line(mat, cv::Point(point.x - mat.cols / 30, point.y + mat.cols / 30), cv::Point(point.x + mat.cols / 30, point.y - mat.cols / 30), cv::Scalar(0.0f), std::max(mat.cols / 100, 1), cv::LINE_AA);
}