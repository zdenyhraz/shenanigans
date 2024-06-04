#pragma once
#include "Microservice.hpp"

class LoadImageMicroservice : public Microservice
{
  void Process() override
  {
    const auto& filename = GetParameter<std::string>("file name");
    auto image = cv::imread(GetProjectDirectoryPath(filename).string());
    if (image.empty())
      throw MicroserviceException("Failed to load image '{}'", filename);

    LOG_DEBUG("Loaded image size: {}x{}x{}", image.cols, image.rows, image.channels());

    SetOutputParameter("image", image);
  }

public:
  LoadImageMicroservice()
  {
    GenerateMicroserviceName();
    DefineOutputParameter<cv::Mat>("image");
    DefineParameter<std::string>("file name", "data/ml/object_detection/datasets/cats/cats2.jpg");
  }
};
