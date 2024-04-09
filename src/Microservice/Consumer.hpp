#pragma once
#include "Microservice.hpp"

template <class T>
class Consumer : public virtual Microservice
{
public:
  using Input = T;

  virtual void Process(T& input) = 0;
};
