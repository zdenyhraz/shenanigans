#pragma once
#include "Dataset.hpp"

cv::Mat ImageSegmentationModelTestFunction(const cv::Mat& mat)
{
  cv::Mat out;
  cv::pow(mat, 0.5, out);
  return out;
}

class ImageSegmentationDataset : public Dataset
{
public:
  ImageSegmentationDataset(const std::string& path, cv::Size size)
  {
    i64 n = 32;
    mInputs = torch::empty({n, size.width * size.height});
    mTargets = torch::empty({n, size.width * size.height});
    cv::Mat image = LoadUnitFloatImage<f32>(path);

    for (i64 i = 0; i < n; ++i)
    {
      const auto x = image.cols / 2 + Random::Rand(-1, 1) * (image.cols / 2 - size.width);
      const auto y = image.rows / 2 + Random::Rand(-1, 1) * (image.rows / 2 - size.height);
      cv::Mat input = RoiCrop(image, x, y, size.width, size.height);
      cv::Mat target = ImageSegmentationModelTestFunction(input);

      mInputs[i] = torch::from_blob(input.data, {size.width * size.height});
      mTargets[i] = torch::from_blob(target.data, {size.width * size.height});
    }
  }
};
