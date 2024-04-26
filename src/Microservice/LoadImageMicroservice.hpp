#pragma once
#include "Producer.hpp"
#include "Entrypoint.hpp"

class LoadImageMicroservice : public Entrypoint, public Producer<cv::Mat>
{
public:
  void Process() const override
  {
    auto image = cv::imread(GetProjectDirectoryPath("test/shenanigans/data/baboon.png").string());
    Submit(image);
  }
};
