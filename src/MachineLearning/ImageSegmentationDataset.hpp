#pragma once
#include "Dataset.hpp"

cv::Mat ImageSegmentationModelTestFunction(const cv::Mat& mat)
{
  cv::Mat out;
  cv::pow(mat, 0.5, out);
  return 1. - out;
}

class ImageSegmentationDataset : public Dataset
{
public:
  ImageSegmentationDataset(const std::string& path, cv::Size size)
  {
    i64 nImages = 32;
    i64 nChannels = 1;
    mInputs = torch::empty({nImages, nChannels, size.height, size.width});
    mTargets = torch::empty({nImages, nChannels, size.height, size.width});
    cv::Mat image = LoadUnitFloatImage<f32>(path);

    for (i64 i = 0; i < nImages; ++i)
    {
      const auto x = image.cols / 2 + Random::Rand(-1, 1) * (image.cols / 2 - size.width);
      const auto y = image.rows / 2 + Random::Rand(-1, 1) * (image.rows / 2 - size.height);
      cv::Mat input = RoiCrop(image, x, y, size.width, size.height);
      cv::Mat target = ImageSegmentationModelTestFunction(input);

      mInputs[i] = torch::from_blob(input.data, {nChannels, size.height, size.width});
      mTargets[i] = torch::from_blob(target.data, {nChannels, size.height, size.width});
    }
  }
};
