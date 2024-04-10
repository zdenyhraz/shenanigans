#pragma once
#include "Microservice.hpp"
#include "Consumer.hpp"

template <class T>
class Producer : public virtual Microservice
{
protected:
  void Notify(T& data) const
  {
    for (const auto& output : outputs)
    {
      auto consumer = dynamic_cast<Consumer<T>*>(output.get());
      if (not consumer)
        throw std::runtime_error(fmt::format("Consumer {}({}) cannot consume data from producer {}({})", output->name, output->kind, name, kind));

      consumer->Process(data);
    }
  }

public:
  using Output = T;
};
