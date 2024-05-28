#pragma once
#include "Microservice.hpp"

class Workflow
{
  std::unordered_map<std::string, std::unique_ptr<Microservice>> microservices;

public:
  void Initialize()
  {
    LOG_FUNCTION;
    for (const auto& [name, microservice] : microservices)
      microservice->Initialize();
  }

  void Run() { LOG_FUNCTION; }
};
