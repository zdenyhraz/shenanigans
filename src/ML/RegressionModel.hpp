#pragma once
#include "Model.hpp"
#include "RegressionDataset.hpp"

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

    x = torch::relu(fc1->forward(x));
    x = torch::relu(fc2->forward(x));
    x = torch::relu(fc3->forward(x));
    x = fc4->forward(x);
    return x;
  }

  void Train(const TrainOptions& options, const std::string& pathTrain, const std::string& pathTest) override
  {
    auto datasetTrain = RegressionDataset(pathTrain, kInputSize, kOutputSize).map(torch::data::transforms::Stack<>());
    auto datasetTest = RegressionDataset(pathTest, kInputSize, kOutputSize).map(torch::data::transforms::Stack<>());

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
        PyPlot::Plot("RegressionModel training", {.x = epochs, .ys = {lossesTrainAvg, lossesTestAvg}, .label_ys = {"train loss", "test loss"}});
      }
    }
  }

  static constexpr i64 kInputSize = 1;
  static constexpr i64 kOutputSize = 1;

private:
  torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr}, fc4{nullptr};
};

void RegressionModelTest()
{
  RegressionModel model;
  model.Train({.epochCount = 50, .plotProgress = false}, "128", "16");
  i64 n = 1001;
  torch::Tensor inputTensor = torch::linspace(0, 1, n).reshape({n, 1});
  torch::Tensor targetTensor = RegressionModelTestFunction(inputTensor);
  torch::Tensor outputTensor = model.Forward(inputTensor);

  PyPlot::Plot("RegressionModel predictions", {.x = ToStdVector<f64>(inputTensor), .ys = {ToStdVector<f64>(targetTensor), ToStdVector<f64>(outputTensor)}, .label_ys = {"target", "output"}});
  return;
}
