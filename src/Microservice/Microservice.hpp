#pragma once

template <typename Derived>
class Microservice
{
  struct MicroserviceParameter
  {
    std::type_index type = std::type_index(typeid(void));
    std::any value;
  };

  std::string name;
  std::unordered_map<std::string, MicroserviceParameter> inputs;
  std::unordered_map<std::string, MicroserviceParameter> outputs;

protected:
  virtual void DefineInputParameters() = 0;
  virtual void DefineOutputParameters() = 0;
  virtual void Process() = 0;

  template <typename T>
  void DefineInputParameter(const std::string& paramName)
  {
    inputs[paramName] = MicroserviceParameter(typeid(T));
  }

  template <typename T>
  void DefineOutputParameter(const std::string& paramName)
  {
    outputs[paramName] = MicroserviceParameter(typeid(T));
  }

  virtual ~Microservice() {}

  template <typename T>
  T& GetInputParameter(const std::string& paramName)
  {
    if (not inputs.contains(paramName))
      throw std::runtime_error(fmt::format("Input parameter '{}' not found", paramName));

    auto& value = inputs.at(paramName).value;

    if (not value.has_value())
      throw std::runtime_error(fmt::format("Input parameter '{}' not set", paramName));

    if (typeid(T) != value.type())
      throw std::runtime_error(fmt::format("Input parameter '{}' type mismatch: {} != {}", paramName, typeid(T).name(), inputs.at(paramName).value.type().name()));

    return std::any_cast<T&>(value);
  }

  template <typename T>
  void SetOutputParameter(const std::string& paramName, const T& value)
  {
    if (not inputs.contains(paramName))
      throw std::runtime_error(fmt::format("Output parameter '{}' not found", paramName));

    if (typeid(T) != outputs.at(paramName).value.type())
      throw std::runtime_error(fmt::format("Output parameter '{}' type mismatch: {} != {}", paramName, typeid(T).name(), inputs.at(paramName).value.type().name()));

    outputs.at(paramName).value = value;
  }

public:
  void Initialize()
  {
    static usize idx = 0;
    name = fmt::format("{}:{}", typeid(Derived).name(), idx++);
    DefineInputParameters();
    DefineOutputParameters();
  }

  void Execute()
  try
  {
    return Process();
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("Microservice <{}> error: {}", name, e.what());
  }
};
