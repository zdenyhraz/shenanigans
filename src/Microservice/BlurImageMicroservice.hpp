#pragma once
#include "Microservice.hpp"

class BlurImageMicroservice : public Microservice
{
  void Process() override
  {
    auto image = GetInputParameter<cv::Mat>("image").clone();
    const auto blurAmount = GetParameter<float>("relative blur");
    const auto blurSize = GetNearestOdd(image.rows * blurAmount);
    cv::GaussianBlur(image, image, cv::Size(blurSize, blurSize), 0);
    SetOutputParameter("blurred", image);
  }

public:
  BlurImageMicroservice()
  {
    GenerateMicroserviceName();
    DefineInputParameter<cv::Mat>("image");
    DefineOutputParameter<cv::Mat>("blurred");
    DefineParameter<float>("relative blur", 0.05);
  }
};
