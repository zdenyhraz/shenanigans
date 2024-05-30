#pragma once

struct MicroserviceInputParameter
{
  std::type_index type = std::type_index(typeid(void));
  std::string name;
  std::any* value; // pointer ot avoid data duplication

  uintptr_t GetId() const { return reinterpret_cast<uintptr_t>(this); }

  void Reset() { value = nullptr; }
};

struct MicroserviceOutputParameter
{
  std::type_index type = std::type_index(typeid(void));
  std::string name;
  std::any value;

  uintptr_t GetId() const { return reinterpret_cast<uintptr_t>(this); }

  void Reset() { value.reset(); }
};

class Microservice
{
  friend class Workflow;

  std::string microserviceName;
  bool start;
  bool finish;
  std::unordered_map<std::string, MicroserviceInputParameter> inputParameters;
  std::unordered_map<std::string, MicroserviceOutputParameter> outputParameters;

protected:
  virtual void DefineInputParameters() = 0;
  virtual void DefineOutputParameters() = 0;

  template <typename T>
  void DefineInputParameter(const std::string& paramName)
  {
    inputParameters[paramName] = MicroserviceInputParameter(std::type_index(typeid(T)));
  }

  template <typename T>
  void DefineOutputParameter(const std::string& paramName)
  {
    outputParameters[paramName] = MicroserviceOutputParameter(std::type_index(typeid(T)));
  }

  template <typename T>
  void SetInputParameter(const std::string& paramName, T& value)
  {
    if (not inputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("Input parameter '{}' not found", paramName));

    auto& param = inputParameters.at(paramName);

    if (param.type != std::type_index(typeid(T)))
      throw std::runtime_error(fmt::format("Input parameter '{}' type mismatch: {} != {}", paramName, typeid(T).name(), param.type.name()));

    param.value = &value;
  }

  template <typename T>
  T& GetInputParameter(const std::string& paramName)
  {
    if (not inputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("Input parameter '{}' not found", paramName));

    auto& param = inputParameters.at(paramName);

    if (not param.value)
      throw std::runtime_error(fmt::format("Input parameter '{}' not set (parameter has no connections)", paramName));

    if (not param.value->has_value())
      throw std::runtime_error(fmt::format("Input parameter '{}' not set", paramName));

    if (param.type != std::type_index(typeid(T)))
      throw std::runtime_error(fmt::format("Input parameter '{}' type mismatch: {} != {}", paramName, typeid(T).name(), param.type.name()));

    return std::any_cast<T&>(*param.value);
  }

  template <typename T>
  void SetOutputParameter(const std::string& paramName, const T& value)
  {
    if (not outputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("Output parameter '{}' not found", paramName));

    auto& param = outputParameters.at(paramName);

    if (param.type != std::type_index(typeid(T)))
      throw std::runtime_error(fmt::format("Output parameter '{}' type mismatch: {} != {}", paramName, typeid(T).name(), param.type.name()));

    param.value = value;
  }

  template <typename T>
  T& GetOutputParameter(const std::string& paramName)
  {
    if (not outputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("Output parameter '{}' not found", paramName));

    auto& param = outputParameters.at(paramName);

    if (param.type != std::type_index(typeid(T)))
      throw std::runtime_error(fmt::format("Output parameter '{}' type mismatch: {} != {}", paramName, typeid(T).name(), param.type.name()));

    if (not param.value.has_value())
      throw std::runtime_error(fmt::format("Output parameter '{}' not set", paramName));

    return std::any_cast<T&>(param.value);
  }

  void GenerateMicroserviceName()
  {
    std::string typeName = typeid(*this).name();
    size_t posSpace = typeName.find(' ');
    if (posSpace != std::string::npos)
      typeName = typeName.substr(posSpace + 1);

    size_t posMs = typeName.find("Microservice");
    if (posMs != std::string::npos)
      typeName.erase(posMs, std::string("Microservice").length());

    static int nameid = 1;
    microserviceName = fmt::format("{}.{}", typeName, nameid++);
  }

public:
  virtual ~Microservice() {}

  virtual void Process() = 0;

  const std::string& GetName() { return microserviceName; }

  uintptr_t GetId() const { return reinterpret_cast<uintptr_t>(this); }

  uintptr_t GetStartId() const { return reinterpret_cast<uintptr_t>(&start); }

  uintptr_t GetFinishId() const { return reinterpret_cast<uintptr_t>(&finish); }

  const std::unordered_map<std::string, MicroserviceInputParameter>& GetInputParameters() { return inputParameters; }

  const std::unordered_map<std::string, MicroserviceOutputParameter>& GetOutputParameters() { return outputParameters; }

  MicroserviceInputParameter& GetInputParameter(const std::string& paramName)
  {
    if (not inputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("Input parameter '{}' not found", paramName));

    return inputParameters.at(paramName);
  }

  MicroserviceOutputParameter& GetOutputParameter(const std::string& paramName)
  {
    if (not outputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("Output parameter '{}' not found", paramName));

    return outputParameters.at(paramName);
  }

  void Initialize()
  {
    GenerateMicroserviceName();
    DefineInputParameters();
    DefineOutputParameters();
  }

  void Reset() // call this on repeated workflows
  {
    for (auto& [name, param] : inputParameters)
      param.Reset();

    for (auto& [name, param] : outputParameters)
      param.Reset();
  }
};
