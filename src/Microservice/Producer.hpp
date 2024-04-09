#pragma once
#include "Microservice.hpp"
#include "Consumer.hpp"

template <class T>
class Producer : public virtual Microservice
{
  std::vector<std::reference_wrapper<Consumer<T>>> consumers;

protected:
  void Notify(T& output)
  {
    for (const auto& consumer : consumers)
      consumer.get().Process(output);
  }

public:
  using Output = T;

  void AddConsumer(Consumer<T>& consumer) { consumers.emplace_back(consumer); }
};
