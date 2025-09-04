#pragma once
#include "Microservice/Microservice.hpp"

class DrawObjects : public Microservice
{
  void Process() override
  {
    LOG_SCOPE("DrawObjects");
    auto image = GetInputParameter<cv::Mat>("image").clone();

    cv::normalize(image, image, 0, 255, cv::NORM_MINMAX);
    image.convertTo(image, CV_8U);

    const auto& bboxes = GetInputParameter<std::vector<cv::Rect>>("objects");
    const auto color = cv::Scalar(0, 0, 255);
    const auto thickness = std::clamp(0.002 * image.rows, 1., 100.);

    for (const auto& bbox : bboxes)
      cv::rectangle(image, bbox, color, thickness);
    SetOutputParameter("image", image);
  }

public:
  DrawObjects()
  {
    GenerateMicroserviceName();
    DefineInputParameter<cv::Mat>("image");
    DefineInputParameter<std::vector<cv::Rect>>("objects");
    DefineOutputParameter<cv::Mat>("image");
  }
};
