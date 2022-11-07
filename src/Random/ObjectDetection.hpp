#pragma once

inline void DetectObjects(const cv::Mat& source, i32 blurSize, i32 stddevSize)
{
  cv::Mat image(source.size(), source.type());
  cv::GaussianBlur(source, image, cv::Size(blurSize, blurSize), 0, 0);

  cv::Mat stddevs = cv::Mat::zeros(image.size(), CV_64F);
  for (i32 r = stddevSize / 2; r < image.rows - stddevSize / 2; ++r)
  {
    for (i32 c = stddevSize / 2; c < image.cols - stddevSize / 2; ++c)
    {
      cv::Scalar mean, stddev;
      cv::meanStdDev(RoiCropRef(image, c, r, stddevSize, stddevSize), mean, stddev);
      stddevs.at<f64>(r, c) = stddev[0];
    }
  }

  Saveimg("../data/debug/source.png", source, false, {0, 0}, false);
  Saveimg("../data/debug/blurred.png", image, false, {0, 0}, false);
  Saveimg("../data/debug/stddevs.png", stddevs, false, {0, 0}, true);
}
