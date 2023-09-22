#pragma once
#include "Model.hpp"
#include "RegressionDataset.hpp"

class RegressionModel : public Model
{
public:
  RegressionModel(i32 width)
  {
    fc1 = register_module("fc1", torch::nn::Linear(kInputSize, width));
    fc2 = register_module("fc2", torch::nn::Linear(width, width / 2));
    fc3 = register_module("fc3", torch::nn::Linear(width / 2, width / 4));
    fc4 = register_module("fc4", torch::nn::Linear(width / 4, kOutputSize));
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

  static constexpr i64 kInputSize = 1;
  static constexpr i64 kOutputSize = 1;

private:
  torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr}, fc4{nullptr};
};

void RegressionModelTest(i32 modelWidth)
{
  RegressionModel model(modelWidth);
  auto datasetTrain = RegressionDataset("128", RegressionModel::kInputSize, RegressionModel::kOutputSize);
  auto datasetTest = RegressionDataset("16", RegressionModel::kInputSize, RegressionModel::kOutputSize);
  model.Train({.epochCount = 50, .plotProgress = true}, datasetTrain, datasetTest);
  i64 n = 1001;
  torch::Tensor inputTensor = torch::linspace(0, 1, n).reshape({n, 1});
  torch::Tensor targetTensor = RegressionModelTestFunction(inputTensor);
  torch::Tensor outputTensor = model.Forward(inputTensor);

  Plot::Plot({.name = "RegressionModel predictions",
      .x = ToStdVector<f64>(inputTensor),
      .ys = {ToStdVector<f64>(targetTensor), ToStdVector<f64>(outputTensor)},
      .ylabels = {"target", "output"}});
  return;
}
