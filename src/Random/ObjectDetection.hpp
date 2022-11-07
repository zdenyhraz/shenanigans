#pragma once

inline void DetectObjects(const cv::Mat& image, i32 size)
{
  cv::Mat stddevs = cv::Mat::zeros(image.size(), CV_64F);
  for (i32 r = size / 2 + 1; r < image.rows - size / 2 - 1; ++r)
  {
    for (i32 c = size / 2 + 1; c < image.cols - size / 2 - 1; ++c)
    {
      cv::Scalar mean, stddev;
      cv::meanStdDev(RoiCropRef(image, c, r, size, size), mean, stddev);
      stddevs.at<f64>(r, c) = stddev[0];
    }
  }

  // Saveimg("stddevs", stddevs, true);
  PyPlot::Plot("stddevs", {.z = stddevs});
}
