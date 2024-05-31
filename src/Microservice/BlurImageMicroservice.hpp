#pragma once
#include "Microservice.hpp"

class BlurImageMicroservice : public Microservice
{
  void DefineInputParameters() override { DefineInputParameter<cv::Mat>("image"); }

  void DefineOutputParameters() override
  {
    DefineOutputParameter<cv::Mat>("blurred");
    DefineOutputParameter<bool>("aaaaaaaaaaa");
  }

  void Process() override
  {
    auto image = GetInputParameter<cv::Mat>("image").clone();
    const auto blurSize = GetNearestOdd(image.rows * 0.05);
    cv::GaussianBlur(image, image, cv::Size(blurSize, blurSize), 0);
    SetOutputParameter("blurred", image);
  }
};
