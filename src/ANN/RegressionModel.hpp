#pragma once
#include "Model.hpp"

class RegressionModel : public Model
{
public:
  RegressionModel()
  {
    fc1 = register_module("fc1", torch::nn::Linear(kInputSize, 128));
    fc2 = register_module("fc2", torch::nn::Linear(128, 64));
    fc3 = register_module("fc3", torch::nn::Linear(64, 32));
    fc4 = register_module("fc4", torch::nn::Linear(32, kOutputSize));
  }

  torch::Tensor Forward(torch::Tensor x) override
  {
    // x = torch::dropout(x, /*p=*/0.5, /*train=*/is_training());
    // x = torch::log_softmax(fc3->forward(x), /*dim=*/1);

    x = torch::relu(fc1->forward(x.reshape({x.size(0), kInputSize})));
    x = torch::relu(fc2->forward(x));
    x = torch::relu(fc3->forward(x));
    x = fc4->forward(x);
    return x;
  }

  void Train(const TrainOptions& options) override
  {
    auto datasetTrain = Dataset(128).map(torch::data::transforms::Stack<>());
    auto datasetTest = Dataset(32).map(torch::data::transforms::Stack<>());

    auto dataloaderTrain = torch::data::make_data_loader<torch::data::samplers::SequentialSampler>(datasetTrain, options.batchSize);
    auto dataloaderTest = torch::data::make_data_loader<torch::data::samplers::SequentialSampler>(datasetTest, options.batchSize);
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
          torch::Tensor predictionTrain = Forward(batchTrain.data).reshape({options.batchSize});
          torch::Tensor lossTrain = torch::mse_loss(predictionTrain, batchTrain.target);
          lossTrainAvg += lossTrain.item<float>();
          lossTrain.backward();
          optimizer.step();
          ++batchTrainIndex;
        }
        lossTrainAvg /= batchTrainIndex;
      }

      {
        i64 batchTestIndex = 0;
        for (auto& batchTest : *dataloaderTest)
        {
          torch::NoGradGuard noGrad;
          torch::Tensor predictionTest = Forward(batchTest.data).reshape({options.batchSize});
          torch::Tensor lossTest = torch::mse_loss(predictionTest, batchTest.target);
          lossTestAvg += lossTest.item<float>();
          ++batchTestIndex;
        }
        lossTestAvg /= batchTestIndex;
      }

      epochs.push_back(epochIndex);
      lossesTrainAvg.push_back(lossTrainAvg);
      lossesTestAvg.push_back(lossTestAvg);

      if (options.plotProgress)
        PyPlot::Plot("RegressionModel::Train", {.x = epochs, .ys = {lossesTrainAvg, lossesTestAvg}, .label_ys = {"lossTrainAvg", "lossTestAvg"}});

      if (options.logProgress and epochIndex % (options.epochCount / options.logProgressCount) == 0)
        LOG_DEBUG("Epoch {} | LossTrain {} | LossTest {}", epochIndex, lossTrainAvg, lossTestAvg);

      if (options.saveNetwork and epochIndex % (options.epochCount / options.saveNetworkCount) == 0)
        torch::save(shared_from_this(), fmt::format("../debug/ANN/net_epoch{}.pt", epochIndex));
    }
  }

private:
  static constexpr size_t kInputSize = 1;
  static constexpr size_t kOutputSize = 1;
  torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr}, fc4{nullptr};
};
