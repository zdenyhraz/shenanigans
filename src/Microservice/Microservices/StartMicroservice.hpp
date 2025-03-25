#pragma once
#include "Microservice/Microservice.hpp"

class StartMicroservice : public Microservice
{
  void Process() override {}

public:
  StartMicroservice() { GenerateMicroserviceName(); }
};
