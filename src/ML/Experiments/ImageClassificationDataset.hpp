#pragma once
#include "Dataset.hpp"

class ImageClassificationDataset : public Dataset
{
public:
  ImageClassificationDataset(const std::filesystem::path& path, cv::Size imageSize = cv::Size(-1, -1))
  {
    const auto classCount = GetDirectoryCount(path);
    std::vector<f32> classIndices;
    for (const auto& [classIndex, classDirectory] : std::views::enumerate(std::filesystem::directory_iterator(path)))
    {
      classNames.push_back(classDirectory.path().string());
      for (const auto& imagePath : std::filesystem::directory_iterator(classDirectory))
      {
        auto image = LoadUnitFloatImage<f32>(imagePath.path());
        if (imageSize != cv::Size(-1, -1))
          cv::resize(image, image, imageSize);
        const auto inputTensor = torch::from_blob(image.data, {1, image.rows, image.cols});

        if (classIndices.empty())
          mInputs = inputTensor;
        else
          torch::cat({mInputs, inputTensor});

        classIndices.push_back(classIndex);
      }
    }
    mTargets = torch::one_hot(torch::from_blob(classIndices.data(), {static_cast<i64>(classIndices.size())}), classCount);
  }

  std::vector<std::string> classNames;
};
