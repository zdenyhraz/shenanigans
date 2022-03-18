#pragma once
#include "Utils.hpp"

class Dataset : public torch::data::Dataset<Dataset>
{
public:
  torch::data::Example<> get(size_t index) override { return {mInputs[index], mTargets[index]}; }

  torch::optional<size_t> size() const override
  {
    //.sizes() array of dimsizes
    //.size(dim) size of dim
    return mInputs.size(0);
  }

protected:
  torch::Tensor mInputs, mTargets;
};
