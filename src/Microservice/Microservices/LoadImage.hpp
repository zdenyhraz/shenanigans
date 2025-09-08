#pragma once
#include "Microservice/Microservice.hpp"
#include "Math/Functions.hpp"

class LoadImage : public Microservice
{
  void Process() override
  {
    LOG_SCOPE("LoadImage");
    const auto imagePath = GetProjectPath(GetParameter<std::string>("image path")).string();
    const bool loadFloat = GetParameter<bool>("convert float");
    const bool loadGrayscale = GetParameter<bool>("grayscale");

    LOG_DEBUG("Loading image '{}'", imagePath);
    int mode = loadGrayscale ? cv::IMREAD_GRAYSCALE : cv::IMREAD_COLOR_BGR;
    auto image = cv::imread(imagePath, mode);
    if (image.empty())
      throw MicroserviceException("Failed to load image '{}'", imagePath);

    if (loadFloat)
    {
      image.convertTo(image, GetMatType<float>());
      cv::normalize(image, image, 0, 1, cv::NORM_MINMAX);
    }

    LOG_DEBUG("Loaded image size: {}x{}x{}", image.cols, image.rows, image.channels());

    SetOutputParameter("image", image);
  }

public:
  LoadImage()
  {
    GenerateMicroserviceName();
    DefineOutputParameter<cv::Mat>("image");
    DefineParameter<std::string>("image path", "data/ml/object_detection/datasets/cats/cats2.jpg");
    DefineParameter<bool>("convert float", false);
    DefineParameter<bool>("grayscale", false);
  }
};
