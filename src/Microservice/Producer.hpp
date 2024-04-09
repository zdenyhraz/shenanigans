#pragma once
#include "Microservice.hpp"
#include "Consumer.hpp"

template <class T>
class Producer : public virtual Microservice
{
protected:
  void Notify(T& data)
  {
    for (const auto& output : outputs)
      dynamic_cast<Consumer<T>*>(output.get())->Process(data);
  }

public:
  using Output = T;
};
