#pragma once
#include "Dataset.hpp"

class Model : public torch::nn::Module
{
public:
  struct TrainOptions
  {
    i64 epochCount = 500;
    i64 batchSize = 16;
    f32 learningRate = 0.001;
    bool saveNetwork = false;
    i64 saveNetworkCount = 5;
    bool logProgress = true;
    i64 logProgressCount = 10;
  };

  virtual torch::Tensor Forward(torch::Tensor x) = 0;
  virtual void Train(const TrainOptions& options) = 0;

private:
};
