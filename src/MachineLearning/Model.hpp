#pragma once
#include "Dataset.hpp"
#include "Utils.hpp"

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

  virtual void Train(const TrainOptions& options, const std::string& pathTrain, const std::string& pathTest) = 0;

private:
};
