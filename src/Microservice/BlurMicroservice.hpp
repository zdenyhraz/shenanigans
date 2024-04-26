#pragma once
#include "Consumer.hpp"
#include "Producer.hpp"

class BlurMicroservice : public Consumer<cv::Mat>, public Producer<cv::Mat>, public Producer<int>
{
  using Producer<cv::Mat>::Submit;
  using Producer<int>::Submit;

  void Process(cv::Mat& image) const override
  {
    cv::GaussianBlur(image, image, cv::Size(29, 29), 17);
    Submit(image);
    int a = 9;
    Submit(a);
  }
};
