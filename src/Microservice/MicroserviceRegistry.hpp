#pragma once
#include "Microservices/LoadImageMicroservice.hpp"
#include "Microservices/BlurMicroservice.hpp"
#include "Microservices/ShowImageMicroservice.hpp"

class MicroserviceRegistry
{
  inline static std::unordered_map<std::string, std::function<std::shared_ptr<Microservice>()>> microserviceFactory;

public:
  static void Initialize() { RegisterMicroservices(); }

  template <class T>
  static void RegisterMicroservice(const std::string& kind)
  {
    static_assert(std::is_base_of_v<Microservice, T>);
    microserviceFactory[kind] = []() { return std::make_shared<T>(); };
  }

  static void RegisterMicroservices()
  {
    // TODO!: add specific microservice parameters
    microserviceFactory.clear();
    RegisterMicroservice<LoadImageMicroservice>("load_image");
    RegisterMicroservice<BlurMicroservice>("blur_image");
    RegisterMicroservice<ShowImageMicroservice>("show_image");
  }

  static std::shared_ptr<Microservice> CreateMicroservice(const std::string& kind, const std::string& name)
  {
    if (not microserviceFactory.contains(kind))
      throw std::invalid_argument(fmt::format("Microservice of kind '{}' not registered", kind));
    auto ms = microserviceFactory[kind]();
    ms->kind = kind;
    ms->name = name;
    return ms;
  }
};
