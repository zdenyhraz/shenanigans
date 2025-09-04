#pragma once
#include "Microservice/Microservice.hpp"

class BlurImage : public Microservice
{
  void Process() override
  {
    auto image = GetInputParameter<cv::Mat>("image").clone();
    const auto blurAmountX = GetParameter<float>("relative blur x");
    const auto blurAmountY = GetParameter<float>("relative blur y");

    const auto blurSizeX = GetNearestOdd(image.rows * blurAmountX);
    const auto blurSizeY = GetNearestOdd(image.rows * blurAmountY);
    cv::GaussianBlur(image, image, cv::Size(blurSizeX, blurSizeY), 0);
    SetOutputParameter("image", image);
  }

public:
  BlurImage()
  {
    GenerateMicroserviceName();
    DefineInputParameter<cv::Mat>("image");
    DefineOutputParameter<cv::Mat>("image");
    DefineParameter<float>("relative blur x", 0.05);
    DefineParameter<float>("relative blur y", 0.15);
    DefineParameter<bool>("median", false);
  }
};
