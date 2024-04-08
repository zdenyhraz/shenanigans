#pragma once
#include "Consumer.hpp"
#include "Producer.hpp"

class BlurMicroservice : public Microservice, public Consumer<cv::Mat>, public Producer<cv::Mat>
{
  void Consume(cv::Mat& image) override
  {
    cv::GaussianBlur(image, image, cv::Size(29, 29), 17);
    Produce(image);
  }

public:
  BlurMicroservice(const std::string& name) : Microservice(name) {}
};
