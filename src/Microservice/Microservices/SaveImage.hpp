#pragma once
#include "Microservice/Microservice.hpp"
#include "Math/Functions.hpp"

class SaveImage : public Microservice
{
  void Process() override
  {
    LOG_SCOPE("SaveImage");
    const auto path = GetProjectPath(GetParameter<std::string>("image path")).string();
    const auto& image = GetInputParameter<cv::Mat>("image");
    cv::imwrite(path, image);
    LOG_DEBUG("Saved image size: {}x{}x{}", image.cols, image.rows, image.channels());
    LOG_DEBUG("Saved image to '{}'", path);
  }

public:
  SaveImage()
  {
    GenerateMicroserviceName();
    DefineInputParameter<cv::Mat>("image");
    DefineParameter<std::string>("image path", "");
  }
};
