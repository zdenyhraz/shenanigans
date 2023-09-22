#pragma once
#include "Model.hpp"
#include "ImageClassificationDataset.hpp"

class ImageClassificationModel : public Model
{
public:
  ImageClassificationModel(usize classCount)
  {
    conv1 = register_module("conv1", torch::nn::Conv2d(kImageChannels, 16, kKernelSize));
    conv2 = register_module("conv2", torch::nn::Conv2d(16, 32, kKernelSize));
    conv3 = register_module("conv3", torch::nn::Conv2d(32, 64, kKernelSize));
    conv4 = register_module("conv4", torch::nn::Conv2d(64, 128, kKernelSize));
    conv5 = register_module("conv5", torch::nn::Conv2d(128, 256, kKernelSize));

    pool = register_module("pool", torch::nn::MaxPool2d(torch::nn::MaxPool2dOptions({2, 2})));

    fc1 = register_module("fc1", torch::nn::Linear(256 * kKernelSize * kKernelSize, 128));
    fc2 = register_module("fc2", torch::nn::Linear(128, 64));
    fc3 = register_module("fc3", torch::nn::Linear(64, classCount));
  }

  torch::Tensor Forward(torch::Tensor x) override
  {
    x = pool(torch::relu(conv1->forward(x)));
    x = pool(torch::relu(conv2->forward(x)));
    x = pool(torch::relu(conv3->forward(x)));
    x = pool(torch::relu(conv4->forward(x)));
    x = pool(torch::relu(conv5->forward(x)));
    x = torch::flatten(x, 1);
    x = torch::relu(fc1->forward(x));
    x = torch::relu(fc2->forward(x));
    x = fc3->forward(x);
    return x;
  }

  static constexpr i64 kImageChannels = 1;
  static constexpr i32 kKernelSize = 5;

private:
  torch::nn::Conv2d conv1{nullptr}, conv2{nullptr}, conv3{nullptr}, conv4{nullptr}, conv5{nullptr};
  torch::nn::MaxPool2d pool{nullptr};
  torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr};
};

void ImageClassificationModelTest()
{
  auto datasetTrain = ImageClassificationDataset(GetProjectDirectoryPath("data/DNN/objdetect/HISAS"));
  auto datasetTest = ImageClassificationDataset("none bro xd");

  ImageClassificationModel model(datasetTrain.classNames.size());
  model.Train({.epochCount = 20, .batchSize = 8}, datasetTrain, datasetTest);
}
