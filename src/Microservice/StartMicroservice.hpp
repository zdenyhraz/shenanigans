#pragma once
#include "Microservice.hpp"

class StartMicroservice : public Microservice
{
  void DefineInputParameters() override {}
  void DefineOutputParameters() override { DefineOutputParameter<bool>("start"); }
  void Process() override { SetOutputParameter("start", true); }
};
