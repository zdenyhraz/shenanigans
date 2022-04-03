#pragma once
#include "PhaseCorrelation.hpp"

class PhaseCorrelationUpscale
{
public:
  using Float = f64;
  inline static f64 mUC = 5;

  static cv::Point2d Calculate(const cv::Mat& image1, const cv::Mat& image2)
  {
    PROFILE_FUNCTION;
    return Calculate(image1.clone(), image2.clone());
  }

  static cv::Point2d Calculate(cv::Mat&& image1, cv::Mat&& image2)
  {
    cv::resize(image1, image1, cv::Size(image1.cols * mUC, image1.cols * mUC));
    cv::resize(image2, image2, cv::Size(image2.cols * mUC, image2.cols * mUC));
    return PhaseCorrelation::Calculate(std::move(image1), std::move(image2)) / mUC;
  }
};
