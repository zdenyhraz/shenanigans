#pragma once
#include "Consumer.hpp"
#include "Producer.hpp"

class BlurMicroservice : public Consumer<cv::Mat>, public Producer<cv::Mat>
{
  void Process(cv::Mat& image) override
  {
    cv::GaussianBlur(image, image, cv::Size(29, 29), 17);
    Notify(image);
  }
};
