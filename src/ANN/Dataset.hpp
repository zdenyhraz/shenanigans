#pragma once

torch::Tensor TestFunction(torch::Tensor x)
{
  return torch::exp(-20. * torch::pow(x - 0.25, 2)) + 3. * torch::exp(-100. * torch::pow(x - 0.75, 2));
}

class Dataset : public torch::data::Dataset<Dataset>
{
public:
  Dataset(size_t n)
  {
    mInputs = torch::rand(n);
    mTargets = TestFunction(mInputs);
  }

  torch::data::Example<> get(size_t index) override { return {mInputs[index], mTargets[index]}; }

  torch::optional<size_t> size() const override
  {
    //.sizes() array of dimsizes
    //.size(dim) size of dim
    return mInputs.size(0);
  }

private:
  torch::Tensor mInputs, mTargets;
};
