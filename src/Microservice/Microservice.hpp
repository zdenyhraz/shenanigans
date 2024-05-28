#pragma once

class Microservice
{
  struct MicroserviceParameter
  {
    std::type_index type = std::type_index(typeid(void));
    std::any value;

    template <typename T>
    bool CheckType()
    {
      return std::type_index(typeid(T)) == type;
    }
  };

  std::string microserviceName;
  std::unordered_map<std::string, MicroserviceParameter> inputParameters;
  std::unordered_map<std::string, MicroserviceParameter> outputParameters;

protected:
  virtual void DefineInputParameters() = 0;
  virtual void DefineOutputParameters() = 0;
  virtual void Process() = 0;

  template <typename T>
  void DefineInputParameter(const std::string& paramName)
  {
    inputParameters[paramName] = MicroserviceParameter(typeid(T));
  }

  template <typename T>
  void DefineOutputParameter(const std::string& paramName)
  {
    outputParameters[paramName] = MicroserviceParameter(typeid(T));
  }

  template <typename T>
  T& GetInputParameter(const std::string& paramName)
  {
    if (not inputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("Input parameter '{}' not found", paramName));

    auto& param = inputParameters.at(paramName);

    if (not param.value.has_value())
      throw std::runtime_error(fmt::format("Input parameter '{}' not set", paramName));

    if (not param.CheckType<T>())
      throw std::runtime_error(fmt::format("Input parameter '{}' type mismatch: {} != {}", paramName, typeid(T).name(), param.value.type().name()));

    return std::any_cast<T&>(param.value);
  }

  template <typename T>
  void SetOutputParameter(const std::string& paramName, const T& value)
  {
    if (not outputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("Output parameter '{}' not found", paramName));

    auto& param = outputParameters.at(paramName);

    if (not param.CheckType<T>())
      throw std::runtime_error(fmt::format("Output parameter '{}' type mismatch: {} != {}", paramName, typeid(T).name(), param.value.type().name()));

    param.value = value;
  }

public:
  virtual ~Microservice() {}

  void Initialize()
  {
    static usize idx = 0;
    microserviceName = fmt::format("{}:{}", typeid(*this).name(), idx++);
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
    LOG_ERROR("Microservice <{}> error: {}", microserviceName, e.what());
  }
};
