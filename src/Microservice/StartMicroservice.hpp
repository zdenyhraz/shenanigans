#pragma once
#include "Microservice.hpp"

class StartMicroservice : public Microservice
{
  void Process() override {}

public:
  StartMicroservice() : Microservice() { GenerateMicroserviceName(); }
};
