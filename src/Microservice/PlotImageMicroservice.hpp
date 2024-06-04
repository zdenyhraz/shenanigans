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
  PlotImageMicroservice()
  {
    GenerateMicroserviceName();
    DefineInputParameter<cv::Mat>("image");
    DefineParameter<bool>("grayscale", false);
  }
};
