#pragma once
#include "Model.hpp"
#include "ImageSegmentationDataset.hpp"

class ImageSegmentationModel : public Model
{
public:
  ImageSegmentationModel()
  {
    conv1 = register_module("conv1", torch::nn::Conv2d(kImageChannels, 16, kKernelSize));
    conv2 = register_module("conv2", torch::nn::Conv2d(16, 32, kKernelSize));
    conv3 = register_module("conv3", torch::nn::Conv2d(32, 16, kKernelSize));
    conv4 = register_module("conv4", torch::nn::Conv2d(16, kImageChannels, kKernelSize));
    us = register_module("us", torch::nn::Upsample(torch::nn::UpsampleOptions().size({{kImageHeight, kImageWidth}}).mode(torch::kBilinear).align_corners(false)));
  }

  torch::Tensor Forward(torch::Tensor x) override
  {
    x = torch::relu(conv1->forward(x));
    x = torch::relu(conv2->forward(x));
    x = torch::relu(conv3->forward(x));
    x = conv4->forward(x);
    x = us->forward(x);
    return x;
  }

  static constexpr i64 kImageWidth = 128;
  static constexpr i64 kImageHeight = 128;
  static constexpr i64 kImageChannels = 1;
  static constexpr i32 kKernelSize = 5;

private:
  torch::nn::Conv2d conv1{nullptr}, conv2{nullptr}, conv3{nullptr}, conv4{nullptr};
  torch::nn::Upsample us;
};

void ImageSegmentationModelTest()
{
  ImageSegmentationModel model;
  const auto path = GetProjectDirectoryPath("data/ObjectDetection/cats/cats2.jpg");
  auto datasetTrain = ImageSegmentationDataset(path.string(), {ImageSegmentationModel::kImageWidth, ImageSegmentationModel::kImageHeight}, 64);
  auto datasetTest = ImageSegmentationDataset(path.string(), {ImageSegmentationModel::kImageWidth, ImageSegmentationModel::kImageHeight}, 16);

  model.Train({.epochCount = 20, .batchSize = 8}, datasetTrain, datasetTest);

  cv::Mat image = LoadUnitFloatImage<f32>(path);
  const cv::Size size(image.rows / 4, image.rows / 4);
  const i32 x = image.cols / 2 + Random::Rand(-1., 1.) * (image.cols / 2 - size.width);
  const i32 y = image.rows / 2 + Random::Rand(-1., 1.) * (image.rows / 2 - size.height);
  cv::Mat input = RoiCrop(image, x, y, size.width, size.height);
  cv::resize(input, input, {ImageSegmentationModel::kImageWidth, ImageSegmentationModel::kImageHeight});
  cv::Mat target = ImageSegmentationModelTestFunction(input);
  torch::Tensor inputTensor = ToTensor(input).reshape({1, ImageSegmentationModel::kImageChannels, ImageSegmentationModel::kImageHeight, ImageSegmentationModel::kImageWidth});
  torch::Tensor outputTensor = model.Forward(inputTensor);

  Plot::Plot("segmentation input", input);
  Plot::Plot("segmentation target", target);
  Plot::Plot("segmentation output", ToCVMat(outputTensor, input.size()));
}
