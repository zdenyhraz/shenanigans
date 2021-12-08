#pragma once
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"

struct filterSettings
{
  double contrast;
  double brightness;
  double gamma;

  filterSettings(double contrast_, double brightness_, double gamma_) : contrast(contrast_), brightness(brightness_), gamma(gamma_) {}
};

cv::Mat filterContrastBrightness(const cv::Mat& sourceimg, double contrast, double brightness);

cv::Mat histogramEqualize(const cv::Mat& sourceimgIn);

cv::Mat gammaCorrect(const cv::Mat& sourceimgIn, double gamma);

cv::Mat addnoise(const cv::Mat& sourceimgIn);

void addnoise(cv::Mat& img, double stddev);

void showhistogram(const cv::Mat& sourceimgIn, int channels, int minimum = 0, int maximum = 255, std::string winname = "histogram");
