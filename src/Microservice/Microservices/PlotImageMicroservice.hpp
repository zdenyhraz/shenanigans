#pragma once
#include "Microservice/Microservice.hpp"
#include "Plot/Plot.hpp"

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
    DefineInputParameter<bool>("normalize");
    DefineParameter<bool>("grayscale", false);
  }
};
