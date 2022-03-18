#pragma once

torch::Tensor TestFunction(torch::Tensor x)
{
  return torch::exp(-20. * torch::pow(x - 0.3, 2)) + 3. * torch::exp(-100. * torch::pow(x - 0.75, 2)) + 0.2 * torch::exp(-50. * torch::pow(x - 0.3, 2)) * torch::sin(x * 6.28 * 50);
}

class Dataset : public torch::data::Dataset<Dataset>
{
public:
  Dataset(const std::string& n)
  {
    mInputs = torch::rand(std::stoi(n));
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
