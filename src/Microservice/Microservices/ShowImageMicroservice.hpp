#pragma once
#include "Consumer.hpp"

class ShowImageMicroservice : public Microservice, public Consumer<cv::Mat>
{
  void Consume(cv::Mat& image) override { Plot::Plot("baboon", image); }

public:
  ShowImageMicroservice(const std::string& name) : Microservice(name) {}
};
