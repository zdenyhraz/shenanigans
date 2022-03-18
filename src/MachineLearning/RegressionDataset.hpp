#pragma once
#include "Dataset.hpp"

torch::Tensor RegressionModelTestFunction(torch::Tensor x)
{
  return torch::exp(-20. * torch::pow(x - 0.3, 2)) + 3. * torch::exp(-100. * torch::pow(x - 0.7, 2)) + 0.2 * torch::exp(-50. * torch::pow(x - 0.3, 2)) * torch::sin(x * 6.28 * 50);
}

class RegressionDataset : public Dataset
{
public:
  RegressionDataset(const std::string& ns, i64 inputSize, i64 outputSize)
  {
    i64 n = std::stoi(ns);
    mInputs = torch::empty({n, inputSize});
    mTargets = torch::empty({n, outputSize});

    for (i64 i = 0; i < n; ++i)
    {
      mInputs[i] = torch::rand(inputSize);
      mTargets[i] = RegressionModelTestFunction(mInputs[i]);
    }
  }
};
