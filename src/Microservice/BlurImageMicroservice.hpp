#pragma once
#include "Microservice.hpp"

class BlurImageMicroservice : public Microservice
{
  void DefineInputParameters() override { DefineInputParameter<cv::Mat>("image"); }

  void DefineOutputParameters() override { DefineOutputParameter<cv::Mat>("blurred_image"); }

  void Process() override
  {
    LOG_DEBUG("{} processing", GetName());
    auto& image = GetInputParameter<cv::Mat>("image");
    const auto blurSize = 7;
    cv::GaussianBlur(image, image, cv::Size(blurSize, blurSize), 0);
    SetOutputParameter("blurred_image", image);
  }
};
