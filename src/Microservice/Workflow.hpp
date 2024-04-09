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
    RegisterMicroservice<LoadImageMircroservice>("load_image");
    RegisterMicroservice<BlurMicroservice>("blur");
    RegisterMicroservice<ShowImageMicroservice>("show_image");
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
      if (not microserviceFactory.contains(node.kind))
        throw std::invalid_argument(fmt::format("Microservice of kind '{}' not registered", node.kind));

      microservices[node.name] = microserviceFactory[node.kind]();
    }

    // step 2: add the input / output links
    for (const auto& node : nodes)
    {
      if (node.inputs.empty() and node.outputs.empty())
        LOG_WARNING("Node '{}' () - node does not have any input or output connections", node.name, node.kind);

      // TODO: two-step approach: initialize all nodes from factory and then build them
      // TODO: make this mandatory vritual func, probably in Microservice class
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
    LoadImageMircroservice load;
    BlurMicroservice blur1;
    BlurMicroservice blur2;
    BlurMicroservice blur3;
    ShowImageMicroservice show;

    load.AddConsumer(blur1);
    blur1.AddConsumer(blur2);
    blur2.AddConsumer(blur3);
    blur3.AddConsumer(show);

    load.Process();
  }
};
