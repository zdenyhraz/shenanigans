#pragma once

struct MicroserviceStartParameter
{
};

struct MicroserviceFinishParameter
{
};

struct MicroserviceInputParameter
{
  std::type_index type = std::type_index(typeid(void));
  std::any* value; // pointer to avoid data duplication
  std::string name;

  uintptr_t GetId() const { return reinterpret_cast<uintptr_t>(this); }
  const std::string& GetName() const { return name; }
  void Reset() { value = nullptr; }
};

struct MicroserviceOutputParameter
{
  std::type_index type = std::type_index(typeid(void));
  std::any value;
  std::string name;

  uintptr_t GetId() const { return reinterpret_cast<uintptr_t>(this); }
  const std::string& GetName() const { return name; }
  void Reset() { value.reset(); }
};

class Microservice
{
  friend class Workflow;

  std::string microserviceName;
  MicroserviceInputParameter start = MicroserviceInputParameter{.type = std::type_index(typeid(MicroserviceStartParameter)), .name = "start"};
  MicroserviceOutputParameter finish = MicroserviceOutputParameter{.type = std::type_index(typeid(MicroserviceFinishParameter)), .name = "finish"};
  std::unordered_map<std::string, MicroserviceInputParameter> inputParameters;
  std::unordered_map<std::string, MicroserviceOutputParameter> outputParameters;

protected:
  virtual void DefineInputParameters() = 0;
  virtual void DefineOutputParameters() = 0;

  template <typename T>
  void DefineInputParameter(const std::string& paramName)
  {
    inputParameters[paramName] = MicroserviceInputParameter{.type = std::type_index(typeid(T)), .name = paramName};
  }

  template <typename T>
  void DefineOutputParameter(const std::string& paramName)
  {
    outputParameters[paramName] = MicroserviceOutputParameter{.type = std::type_index(typeid(T)), .name = paramName};
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

  uintptr_t GetStartId() const { return start.GetId(); }

  uintptr_t GetFinishId() const { return finish.GetId(); }

  std::unordered_map<std::string, MicroserviceInputParameter>& GetInputParameters() { return inputParameters; }

  std::unordered_map<std::string, MicroserviceOutputParameter>& GetOutputParameters() { return outputParameters; }

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
