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
  ImageSegmentationDataset(const std::string& path, cv::Size imageSize, i64 imageCount)
  {
    i64 nChannels = 1;
    mInputs = torch::empty({imageCount, nChannels, imageSize.height, imageSize.width});
    mTargets = torch::empty({imageCount, nChannels, imageSize.height, imageSize.width});
    cv::Mat image = LoadUnitFloatImage<f32>(path);

    for (i64 i = 0; i < imageCount; ++i)
    {
      const auto x = image.cols / 2 + Random::Rand(-1, 1) * (image.cols / 2 - imageSize.width);
      const auto y = image.rows / 2 + Random::Rand(-1, 1) * (image.rows / 2 - imageSize.height);
      cv::Mat input = RoiCropRep<f32>(image, x, y, image.cols * 0.7, image.rows * 0.7);
      cv::resize(input, input, cv::Size(imageSize.width, imageSize.height));
      cv::Mat target = ImageSegmentationModelTestFunction(input);

      mInputs[i] = torch::from_blob(input.data, {nChannels, imageSize.height, imageSize.width});
      mTargets[i] = torch::from_blob(target.data, {nChannels, imageSize.height, imageSize.width});
    }
  }
};
