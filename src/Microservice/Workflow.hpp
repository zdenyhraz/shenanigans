#pragma once
#include "Node.hpp"
#include "Consumer.hpp"
#include "Producer.hpp"
#include "Entrypoint.hpp"
#include "Microservice.hpp"
#include "MicroserviceRegistry.hpp"

class Workflow
{
  std::unordered_map<std::string, std::shared_ptr<Microservice>> microservices;
  std::string name;

public:
  Workflow(const std::string name) : name(name) {}

  void Build(const std::vector<Node>& nodes)
  {
    LOG_FUNCTION;

    if (nodes.empty())
      throw std::invalid_argument(fmt::format("Cannot build workflow '{}' - node vector empty", name));

    // step 1: create the microservices
    for (const auto& node : nodes)
    {
      if (microservices.contains(node.name))
        throw std::invalid_argument(fmt::format("Duplicate microservice name '{}'", node.name));

      auto ms = MicroserviceRegistry::CreateMicroservice(node.kind);
      ms->name = node.name;
      ms->kind = node.kind;
      microservices[node.name] = ms;
    }

    // step 2: add the input / output microservice links
    for (const auto& node : nodes)
    {
      auto ms = microservices[node.name];

      if (node.inputs.empty() and node.outputs.empty())
        LOG_WARNING("Node {}({}) does not have any input or output connections", node.name, node.kind);

      if (node.inputs.empty() and not dynamic_cast<Entrypoint*>(ms.get()))
        throw std::runtime_error(fmt::format("Node {}({}) does not have any input connections and is not an entrypoint", node.name, node.kind));

      for (const auto& inputNode : node.inputs)
        ms->AddInput(microservices[inputNode->name]);
      for (const auto& outputNode : node.outputs)
        ms->AddOutput(microservices[outputNode->name]);
    }
  }

  void Run()
  {
    LOG_FUNCTION;
    for (const auto& [name, ms] : microservices | std::views::filter([](const auto& pair) { return pair.second->inputs.empty(); }))
      dynamic_cast<Entrypoint*>(ms.get())->Process();
  }

  static void TestManual()
  {
    LOG_FUNCTION;
    auto load = MicroserviceRegistry::CreateMicroservice("load_image");
    auto blur1 = MicroserviceRegistry::CreateMicroservice("blur_image");
    auto blur2 = MicroserviceRegistry::CreateMicroservice("blur_image");
    auto blur3 = MicroserviceRegistry::CreateMicroservice("blur_image");
    auto show = MicroserviceRegistry::CreateMicroservice("show_image");

    load->AddOutput(blur1);
    blur1->AddOutput(blur2);
    blur2->AddOutput(blur3);
    blur3->AddOutput(show);

    dynamic_cast<LoadImageMicroservice*>(load.get())->Process();
  }
};
