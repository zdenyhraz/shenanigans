#pragma once
#include "Model.hpp"
#include "ImageSegmentationDataset.hpp"

class ImageSegmentationModel : public Model
{
public:
  ImageSegmentationModel()
  {
    fc1 = register_module("fc1", torch::nn::Linear(kImageWidth * kImageHeight, 1024));
    fc2 = register_module("fc2", torch::nn::Linear(1024, 512));
    fc3 = register_module("fc3", torch::nn::Linear(512, 1024));
    fc4 = register_module("fc4", torch::nn::Linear(1024, kImageWidth * kImageHeight));
  }

  torch::Tensor Forward(torch::Tensor x) override
  {
    x = torch::relu(fc1->forward(x));
    x = torch::relu(fc2->forward(x));
    x = torch::relu(fc3->forward(x));
    x = fc4->forward(x);
    return x;
  }

  void Train(const TrainOptions& options, const std::string& pathTrain, const std::string& pathTest) override
  {
    auto datasetTrain = ImageSegmentationDataset(pathTrain, {kImageWidth, kImageHeight}).map(torch::data::transforms::Stack<>());
    auto datasetTest = ImageSegmentationDataset(pathTest, {kImageWidth, kImageHeight}).map(torch::data::transforms::Stack<>());

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
        PyPlot::Plot("ImageSegmentationModel training", {.x = epochs, .ys = {lossesTrainAvg, lossesTestAvg}, .label_ys = {"train loss", "test loss"}});
      }
    }
  }

  static constexpr i64 kImageWidth = 128;
  static constexpr i64 kImageHeight = 128;

private:
  torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr}, fc4{nullptr};
};

void ImageSegmentationModelTest()
{
  ImageSegmentationModel model;
  model.Train({.epochCount = 10, .batchSize = 2}, "../debug/AIA/171A.png", "../debug/AIA/171A.png");

  cv::Mat image = LoadUnitFloatImage<f32>("../debug/AIA/171A.png");
  const i32 x = image.cols / 2 + Random::Rand(-1., 1.) * (image.cols / 2 - ImageSegmentationModel::kImageWidth);
  const i32 y = image.rows / 2 + Random::Rand(-1., 1.) * (image.rows / 2 - ImageSegmentationModel::kImageHeight);

  cv::Mat input = RoiCrop(image, x, y, ImageSegmentationModel::kImageWidth, ImageSegmentationModel::kImageHeight);
  cv::Mat target = ImageSegmentationModelTestFunction(input);
  torch::Tensor inputTensor = ToTensor(input);
  torch::Tensor outputTensor = model.Forward(inputTensor);

  PyPlot::Plot("ImageSegmentationModel input", {.z = input});
  PyPlot::Plot("ImageSegmentationModel target", {.z = target});
  PyPlot::Plot("ImageSegmentationModel output", {.z = ToCVMat(outputTensor, input.size())});
}
