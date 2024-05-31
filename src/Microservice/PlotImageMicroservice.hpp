#pragma once
#include "Microservice.hpp"

class PlotImageMicroservice : public Microservice
{
  void DefineInputParameters() override { DefineInputParameter<cv::Mat>("image"); }

  void DefineOutputParameters() override {}

  void Process() override
  {
    auto& image = GetInputParameter<cv::Mat>("image");
    Plot::Plot(GetName(), image);
  }
};
