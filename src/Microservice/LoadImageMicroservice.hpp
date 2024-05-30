#pragma once
#include "Microservice.hpp"

class LoadImageMicroservice : public Microservice
{
  void DefineInputParameters() override {}

  void DefineOutputParameters() override { DefineOutputParameter<cv::Mat>("image"); }

  void Process() override
  {
    LOG_DEBUG("{} processing", GetName());
    auto image = cv::imread(GetProjectDirectoryPath("data/ml/object_detection/datasets/cats/cats2.jpg").string());
    LOG_DEBUG("Loaded image size: {}x{}x{}", image.cols, image.rows, image.channels());
    SetOutputParameter("image", image);
  }
};
