#pragma once
#include "Microservice.hpp"
#include "Consumer.hpp"
#include "Utils/String.hpp"

template <class T>
class Producer : public virtual Microservice
{
protected:
  void Submit(T& data) const
  {
    bool processed = false;
    for (const auto& output : outputs)
    {
      const auto consumer = dynamic_cast<Consumer<T>*>(output.get());
      if (consumer)
      {
        consumer->Process(data);
        processed = true;
      }
    }

    if (not processed)
      LOG_WARNING("No consumer available to consume <{}> from producer {}<{}>", ExtractSubstring(std::source_location::current().function_name(), "Producer<", ">"), name, kind);
  }

public:
  using Output = T;
};
