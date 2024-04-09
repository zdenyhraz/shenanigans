#pragma once

class Microservice
{
public:
  std::string name;
  std::string kind;
  std::vector<std::shared_ptr<Microservice>> inputs;
  std::vector<std::shared_ptr<Microservice>> outputs;

  virtual ~Microservice() = default;
  void AddInput(std::shared_ptr<Microservice> input) { inputs.emplace_back(input); }
  void AddOutput(std::shared_ptr<Microservice> output) { outputs.emplace_back(output); }
};
