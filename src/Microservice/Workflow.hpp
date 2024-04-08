#pragma once
#include "Microservice.hpp"
#include "Microservices/LoadImageMicroservice.hpp"
#include "Microservices/BlurMicroservice.hpp"
#include "Microservices/ShowImageMicroservice.hpp"

class Workflow
{
  inline static std::unordered_map<std::string, std::function<std::unique_ptr<Microservice>(const std::string&)>> microserviceFactory;
  std::vector<std::unique_ptr<Microservice>> microservices;

  template <class T>
  static void RegisterMicroservice(const std::string& name)
  {
    static_assert(std::is_base_of_v<Microservice, T>);
    microserviceFactory[name] = [](const std::string& name) { return std::make_unique<T>(name); };
  }

  static void RegisterMicroservices()
  {
    // TODO!: specific microservice parameters
    RegisterMicroservice<LoadImageMircroservice>("load_image");
    RegisterMicroservice<BlurMicroservice>("blur");
    RegisterMicroservice<ShowImageMicroservice>("show_image");
  }

public:
  void Build()
  {
    RegisterMicroservices();
    LoadImageMircroservice load("load");
    BlurMicroservice blur1("blur1");
    BlurMicroservice blur2("blur2");
    BlurMicroservice blur3("blur3");
    ShowImageMicroservice show("show");

    load.AddConsumer(blur1);
    blur1.AddConsumer(blur2);
    blur2.AddConsumer(blur3);
    blur3.AddConsumer(show);

    load.Process(); // TODO!: somehow work out these "entry points" - no input
  }

  void Run() {}
};
