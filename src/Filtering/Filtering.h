#pragma once

struct filterSettings
{
  f64 contrast;
  f64 brightness;
  f64 gamma;

  filterSettings(f64 contrast_, f64 brightness_, f64 gamma_) : contrast(contrast_), brightness(brightness_), gamma(gamma_) {}
};

cv::Mat filterContrastBrightness(const cv::Mat& sourceimg, f64 contrast, f64 brightness);

cv::Mat histogramEqualize(const cv::Mat& sourceimgIn);

cv::Mat gammaCorrect(const cv::Mat& sourceimgIn, f64 gamma);

cv::Mat addnoise(const cv::Mat& sourceimgIn);

void addnoise(cv::Mat& img, f64 stddev);

void showhistogram(const cv::Mat& sourceimgIn, i32 channels, i32 minimum = 0, i32 maximum = 255, std::string winname = "histogram");
