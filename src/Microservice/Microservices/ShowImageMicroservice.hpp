#pragma once
#include "Consumer.hpp"

class ShowImageMicroservice : public Consumer<cv::Mat>
{
  void Process(cv::Mat& image) override { Plot::Plot("baboon", image); }
};
