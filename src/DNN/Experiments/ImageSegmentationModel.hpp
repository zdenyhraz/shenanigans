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

  void Train(const TrainOptions& options, const std::string& pathTrain, const std::string& pathTest) override
  {
    auto datasetTrain = ImageSegmentationDataset(pathTrain, {kImageWidth, kImageHeight}, 64).map(torch::data::transforms::Stack<>());
    auto datasetTest = ImageSegmentationDataset(pathTest, {kImageWidth, kImageHeight}, 16).map(torch::data::transforms::Stack<>());

    auto dataloaderTrain = torch::data::make_data_loader<torch::data::samplers::SequentialSampler>(std::move(datasetTrain), options.batchSize);
    auto dataloaderTest = torch::data::make_data_loader<torch::data::samplers::SequentialSampler>(std::move(datasetTest), options.batchSize);

    torch::optim::Adam optimizer(parameters(), options.learningRate);
    std::vector<f64> epochs, lossesTrainAvg, lossesTestAvg;

    for (i64 epochIndex = 0; epochIndex < options.epochCount; ++epochIndex)
    {
      f32 lossTrainAvg = 0, lossTestAvg = 0;

      {
        i64 batchTrainIndex = 0;
        for (auto& batchTrain : *dataloaderTrain)
        {
          optimizer.zero_grad();
          torch::Tensor predictionTrain = Forward(batchTrain.data);
          torch::Tensor lossTrain = torch::mse_loss(predictionTrain, batchTrain.target);
          lossTrainAvg += lossTrain.item<f32>();
          lossTrain.backward();
          optimizer.step();
          ++batchTrainIndex;
        }
        lossTrainAvg /= batchTrainIndex;
      }

      {
        torch::NoGradGuard noGrad;
        i64 batchTestIndex = 0;
        for (auto& batchTest : *dataloaderTest)
        {
          torch::Tensor predictionTest = Forward(batchTest.data);
          torch::Tensor lossTest = torch::mse_loss(predictionTest, batchTest.target);
          lossTestAvg += lossTest.item<f32>();
          ++batchTestIndex;
        }
        lossTestAvg /= batchTestIndex;
      }

      if (options.logProgress and epochIndex % (options.epochCount / options.logProgressCount) == 0)
        LOG_DEBUG("Epoch {} | TrainLoss {:.2e} | TestLoss {:.2e}", epochIndex, lossTrainAvg, lossTestAvg);

      if (options.saveNetwork and epochIndex % (options.epochCount / options.saveNetworkCount) == 0)
        torch::save(shared_from_this(), fmt::format("../debug/ANN/net_epoch{}.pt", epochIndex));

      if (options.plotProgress)
      {
        epochs.push_back(epochIndex);
        lossesTrainAvg.push_back(lossTrainAvg);
        lossesTestAvg.push_back(lossTestAvg);
        Plot::Plot({.name = "ImageSegmentationModel training", .x = epochs, .ys = {lossesTrainAvg, lossesTestAvg}, .ylabels = {"train loss", "test loss"}, .log = true});
      }
    }
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
  const auto path = (GetProjectDirectoryPath() / "data/dissertation/dissimilar/input2.png").string();
  model.Train({.epochCount = 20, .batchSize = 8}, path, path);

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
