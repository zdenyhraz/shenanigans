#pragma once
#include "Microservice.hpp"

class LoadImageMicroservice : public Microservice
{
  void Process() override
  {
    auto image = cv::imread(GetProjectDirectoryPath("data/ml/object_detection/datasets/cats/cats2.jpg").string());
    LOG_DEBUG("Loaded image size: {}x{}x{}", image.cols, image.rows, image.channels());
    SetOutputParameter("image", image);
  }

public:
  LoadImageMicroservice() : Microservice()
  {
    GenerateMicroserviceName();
    DefineOutputParameter<cv::Mat>("image");
  }
};
