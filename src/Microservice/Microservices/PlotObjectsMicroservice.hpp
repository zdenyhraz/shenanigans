#pragma once
#include "Microservice/Microservice.hpp"

class PlotObjectsMicroservice : public Microservice
{
  void Process() override
  {
    LOG_FUNCTION;
    auto image = GetInputParameter<cv::Mat>("image").clone();

    cv::normalize(image, image, 0, 255, cv::NORM_MINMAX);
    image.convertTo(image, CV_8U);
    cv::Mat out;
    cv::applyColorMap(image, out, cv::COLORMAP_VIRIDIS);

    const auto& bboxes = GetInputParameter<std::vector<cv::RotatedRect>>("objects");
    const auto color = cv::Scalar(0, 0, 255);
    const auto thickness = std::clamp(0.002 * out.rows, 1., 100.);

    for (const auto& bbox : bboxes)
    {
      std::vector<cv::Point2f> bboxpts(4);
      bbox.points(bboxpts.data());
      cv::drawContours(out, std::vector<std::vector<cv::Point>>{std::vector<cv::Point>(bboxpts.begin(), bboxpts.end())}, -1, color, thickness, cv::LINE_AA);
    }

    Plot::Plot(GetName(), out);
  }

public:
  PlotObjectsMicroservice()
  {
    GenerateMicroserviceName();
    DefineInputParameter<cv::Mat>("image");
    DefineInputParameter<std::vector<cv::RotatedRect>>("objects");
  }
};
