#pragma once
#include "Microservice.hpp"

class Entrypoint : public virtual Microservice
{
public:
  virtual void Process() const = 0;
};
