#pragma once
#include "Microservice/Microservice.hpp"
#include "Math/Functions.hpp"

class LoadImageMicroservice : public Microservice
{
  void Process() override
  {
    LOG_FUNCTION;
    const auto& filename = GetParameter<std::string>("file name");
    const bool loadFloat = GetParameter<bool>("float");
    const bool loadGrayscale = GetParameter<bool>("grayscale");

    int mode = 0;
    if (not loadGrayscale)
      mode |= cv::IMREAD_COLOR;

    auto image = cv::imread(GetProjectPath(filename).string(), mode);
    if (image.empty())
      throw MicroserviceException("Failed to load image '{}'", filename);

    if (loadFloat)
    {
      image.convertTo(image, GetMatType<float>());
      cv::normalize(image, image, 0, 1, cv::NORM_MINMAX);
    }

    LOG_DEBUG("Loaded image size: {}x{}x{}", image.cols, image.rows, image.channels());

    SetOutputParameter("image", image);
  }

public:
  LoadImageMicroservice()
  {
    GenerateMicroserviceName();
    DefineOutputParameter<cv::Mat>("image");
    DefineParameter<std::string>("file name", "data/ml/object_detection/datasets/cats/cats2.jpg");
    DefineParameter<bool>("float", false);
    DefineParameter<bool>("grayscale", false);
  }
};
