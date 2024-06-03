#pragma once
#include "Microservice.hpp"

class BlurImageMicroservice : public Microservice
{
  void Process() override
  {
    auto image = GetInputParameter<cv::Mat>("image").clone();
    const auto blurSize = GetNearestOdd(image.rows * 0.05);
    cv::GaussianBlur(image, image, cv::Size(blurSize, blurSize), 0);
    SetOutputParameter("blurred", image);
  }

public:
  BlurImageMicroservice() : Microservice()
  {
    GenerateMicroserviceName();
    DefineInputParameter<cv::Mat>("image");
    DefineOutputParameter<cv::Mat>("blurred");
    DefineOutputParameter<bool>("aaaaaaaaaaa");
  }
};
