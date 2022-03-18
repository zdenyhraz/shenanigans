#pragma once
#include "Dataset.hpp"

class ImageSegmentationDataset : public Dataset
{
public:
  ImageSegmentationDataset(const std::string& n)
  {
    mInputs = torch::rand(std::stoi(n));
    mTargets = torch::rand(std::stoi(n));
  }
};
