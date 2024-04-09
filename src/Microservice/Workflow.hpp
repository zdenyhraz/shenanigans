#pragma once
#include "Node.hpp"
#include "Microservice.hpp"
#include "Microservices/LoadImageMicroservice.hpp"
#include "Microservices/BlurMicroservice.hpp"
#include "Microservices/ShowImageMicroservice.hpp"

class Workflow
{
  inline static std::unordered_map<std::string, std::function<std::shared_ptr<Microservice>()>> microserviceFactory;
  std::unordered_map<std::string, std::shared_ptr<Microservice>> microservices;

  template <class T>
  static void RegisterMicroservice(const std::string& kind)
  {
    static_assert(std::is_base_of_v<Microservice, T>);
    microserviceFactory[kind] = []() { return std::make_unique<T>(); };
  }

  static void RegisterMicroservices()
  {
    // TODO!: add specific microservice parameters
    RegisterMicroservice<LoadImageMicroservice>("load_image");
    RegisterMicroservice<BlurMicroservice>("blur_image");
    RegisterMicroservice<ShowImageMicroservice>("show_image");
  }

  static std::shared_ptr<Microservice> CreateMicroservice(const std::string& kind)
  {
    if (not microserviceFactory.contains(kind))
      throw std::invalid_argument(fmt::format("Microservice of kind '{}' not registered", kind));
    return microserviceFactory[kind]();
  }

public:
  void Build()
  {
    RegisterMicroservices(); // TODO!: static func call, call once somewhere

    std::vector<Node> nodes; // TODO!: import this from json or blueprint GUI or something

    // step 1: create the microservices
    for (const auto& node : nodes)
    {
      if (microservices.contains(node.name))
        throw std::invalid_argument(fmt::format("Duplicate microservice name '{}'", node.name));

      auto ms = CreateMicroservice(node.kind);
      ms->name = node.name;
      ms->kind = node.kind;
      microservices[node.name] = ms;
    }

    // step 2: add the input / output microservice links
    for (const auto& node : nodes)
    {
      if (node.inputs.empty() and node.outputs.empty())
        LOG_WARNING("Node '{}' () - node does not have any input or output connections", node.name, node.kind);

      for (const auto& inputNode : node.inputs)
        microservices[node.name]->AddInput(microservices[inputNode->name]);
      for (const auto& outputNode : node.outputs)
        microservices[node.name]->AddOutput(microservices[outputNode->name]);

      // ms->Build(node);
    }
  }

  void Run()
  {
    // TODO!: somehow work out these "entry points" - no input
    // TODO: call Process() on all entry points
    // for (const auto& microservice : microservices | std::views::filter([](const auto& ms) { return ms. }))
  }

  void TestManual()
  {
    RegisterMicroservices();
    auto load = CreateMicroservice("load_image");
    auto blur1 = CreateMicroservice("blur_image");
    auto blur2 = CreateMicroservice("blur_image");
    auto blur3 = CreateMicroservice("blur_image");
    auto show = CreateMicroservice("show_image");

    load->AddOutput(blur1);
    blur1->AddOutput(blur2);
    blur2->AddOutput(blur3);
    blur3->AddOutput(show);

    dynamic_cast<LoadImageMicroservice*>(load.get())->Process();
  }
};
