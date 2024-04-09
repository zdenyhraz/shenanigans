#pragma once
#include "Node.hpp"

class Microservice
{
  std::string name;

public:
  virtual void Build(const Node& node) // TODO: pure virtual =0 ?
  {
    if (not node.outputs.empty())
    {
      for (const auto& outputNode : node.outputs)
      {
        // microservice.AddConsumer();
      }
    }
  }
};
