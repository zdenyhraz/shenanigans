#pragma once
#include "Producer.hpp"

class LoadImageMircroservice : public Producer<cv::Mat>
{
public:
  void Process() // TODO!: somehow work out these "entry points" - no input
  {
    auto image = cv::imread(GetProjectDirectoryPath("test/data/baboon.png").string());
    Notify(image);
  }
};
