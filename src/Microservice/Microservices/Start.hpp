#pragma once
#include "Microservice/Microservice.hpp"

class Start : public Microservice
{
  void Process() override {}

public:
  Start() { GenerateMicroserviceName(); }
};
