#pragma once
#include "Dataset.hpp"

class Model : public torch::nn::Module
{
public:
  struct TrainOptions
  {
    i64 epochCount = 100;
    i64 batchSize = 16;
    f32 learningRate = 5e-3;
    bool saveNetwork = false;
    i64 saveNetworkCount = 5;
    bool logProgress = true;
    i64 logProgressCount = 10;
    bool plotProgress = true;
  };

  virtual torch::Tensor Forward(torch::Tensor x) = 0;

  template <typename DatasetT>
  void Train(const TrainOptions& options, DatasetT&& datasetTrain, DatasetT&& datasetTest)
  {
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
        Plot::Plot({.name = "RegressionModel training", .x = epochs, .ys = {lossesTrainAvg, lossesTestAvg}, .ylabels = {"train loss", "test loss"}});
      }
    }
  }

private:
};
