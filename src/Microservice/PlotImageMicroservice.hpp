#pragma once
#include "Microservice.hpp"

class PlotImageMicroservice : public Microservice
{
  void Process() override
  {
    auto& image = GetInputParameter<cv::Mat>("image");
    Plot::Plot(GetName(), image);
  }

public:
  PlotImageMicroservice() : Microservice()
  {
    GenerateMicroserviceName();
    DefineInputParameter<cv::Mat>("image");
  }
};
