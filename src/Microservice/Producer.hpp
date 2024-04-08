#pragma once
#include "Consumer.hpp"

template <class Output>
class Producer
{
  std::vector<std::reference_wrapper<Consumer<Output>>> consumers;

protected:
  void Produce(Output& output)
  {
    for (const auto& consumer : consumers)
      consumer.get().Consume(output);
  }

public:
  template <typename T>
  void AddConsumer(Consumer<T>& consumer)
  {
    consumers.emplace_back(consumer);
  }
};
