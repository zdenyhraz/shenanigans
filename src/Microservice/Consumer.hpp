#pragma once
#include "Microservice.hpp"

template <class Input>
class Consumer
{
public:
  virtual void Consume(Input& input) = 0;
};
