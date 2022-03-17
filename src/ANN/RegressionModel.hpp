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
    auto datasetTrain = Dataset(256).map(torch::data::transforms::Stack<>());
    auto datasetTest = Dataset(64).map(torch::data::transforms::Stack<>());

    auto dataloaderTrain = torch::data::make_data_loader<torch::data::samplers::SequentialSampler>(std::move(datasetTrain), options.batchSize);
    auto dataloaderTest = torch::data::make_data_loader<torch::data::samplers::SequentialSampler>(std::move(datasetTest), *datasetTest.size());

    torch::optim::Adam optimizer(parameters(), options.learningRate); // SGD, Adam, ...

    for (i64 epochIndex = 0; epochIndex < options.epochCount; ++epochIndex)
    {
      for (auto& batch : *dataloaderTrain)
      {
        optimizer.zero_grad();
        torch::Tensor prediction = Forward(batch.data).reshape({options.batchSize});
        torch::Tensor loss = torch::mse_loss(prediction, batch.target);
        loss.backward();
        optimizer.step();
      }

      if (options.logProgress and epochIndex % (options.epochCount / options.logProgressCount) == 0)
      {
        // evaluate training and test loss on the entire dataset
        torch::NoGradGuard noGrad;

        torch::Tensor prediction = Forward(dataloaderTrain->begin()->data);
        torch::Tensor predictionTest = Forward(dataloaderTest->begin()->data);

        torch::Tensor loss = torch::mse_loss(prediction, dataloaderTrain->begin()->target);
        torch::Tensor lossTest = torch::mse_loss(predictionTest, dataloaderTest->begin()->target);

        LOG_DEBUG("Epoch {} | LossTrain {} | LossTest {}", epochIndex, loss.item<float>(), lossTest.item<float>());
      }

      if (options.saveNetwork and epochIndex % (options.epochCount / options.saveNetworkCount) == 0)
        torch::save(shared_from_this(), fmt::format("../debug/ANN/net_epoch{}.pt", epochIndex));
    }
  }

private:
  static constexpr size_t kInputSize = 1;
  static constexpr size_t kOutputSize = 1;
  torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr}, fc4{nullptr};
};
