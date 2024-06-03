#pragma once

struct MicroserviceStartFinish
{
};

struct MicroserviceInputParameter
{
  const std::type_info& type;
  std::string name;
  std::any* value; // pointer to avoid data duplication

  MicroserviceInputParameter(const std::type_info& _type, const std::string& _name) : type(_type), name(_name) {}

  uintptr_t GetId() const { return reinterpret_cast<uintptr_t>(this); }
  const std::string& GetName() const { return name; }
  void Reset() { value = nullptr; }
};

struct MicroserviceOutputParameter
{
  const std::type_info& type;
  std::string name;
  std::any value;

  MicroserviceOutputParameter(const std::type_info& _type, const std::string& _name) : type(_type), name(_name) {}

  uintptr_t GetId() const { return reinterpret_cast<uintptr_t>(this); }
  const std::string& GetName() const { return name; }
  void Reset() { value.reset(); }
};

class Microservice
{
  std::string microserviceName;
  MicroserviceInputParameter start = MicroserviceInputParameter{typeid(MicroserviceStartFinish), ""};
  MicroserviceOutputParameter finish = MicroserviceOutputParameter{typeid(MicroserviceStartFinish), ""};
  std::unordered_map<std::string, MicroserviceInputParameter> inputParameters;
  std::unordered_map<std::string, MicroserviceOutputParameter> outputParameters;

protected:
  virtual void DefineInputParameters() = 0;
  virtual void DefineOutputParameters() = 0;

  template <typename T>
  void DefineInputParameter(const std::string& paramName)
  {
    inputParameters.emplace(paramName, MicroserviceInputParameter{typeid(T), paramName});
  }

  template <typename T>
  void DefineOutputParameter(const std::string& paramName)
  {
    outputParameters.emplace(paramName, MicroserviceOutputParameter{typeid(T), paramName});
  }

  template <typename T>
  void SetInputParameter(const std::string& paramName, T& value)
  {
    if (not inputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("{}: Input parameter '{}' not found", GetName(), paramName));

    auto& param = inputParameters.at(paramName);

    if (param.type != std::type_index(typeid(T)))
      throw std::runtime_error(fmt::format("{}: Input parameter '{}' type mismatch: {} != {}", GetName(), paramName, typeid(T).name(), param.type.name()));

    param.value = &value;
  }

  template <typename T>
  T& GetInputParameter(const std::string& paramName)
  {
    if (not inputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("{}: Input parameter '{}' not found", GetName(), paramName));

    auto& param = inputParameters.at(paramName);

    if (not param.value)
      throw std::runtime_error(fmt::format("{}: Input parameter '{}' not set (parameter has no connections)", GetName(), paramName));

    if (not param.value->has_value())
      throw std::runtime_error(fmt::format("{}: Input parameter '{}' not set", GetName(), paramName));

    if (param.type != std::type_index(typeid(T)))
      throw std::runtime_error(fmt::format("{}: Input parameter '{}' type mismatch: {} != {}", GetName(), paramName, typeid(T).name(), param.type.name()));

    return std::any_cast<T&>(*param.value);
  }

  template <typename T>
  void SetOutputParameter(const std::string& paramName, const T& value)
  {
    if (not outputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("{}: Output parameter '{}' not found", GetName(), paramName));

    auto& param = outputParameters.at(paramName);

    if (param.type != std::type_index(typeid(T)))
      throw std::runtime_error(fmt::format("{}: Output parameter '{}' type mismatch: {} != {}", GetName(), paramName, typeid(T).name(), param.type.name()));

    param.value = value;
  }

  template <typename T>
  T& GetOutputParameter(const std::string& paramName)
  {
    if (not outputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("{}: Output parameter '{}' not found", GetName(), paramName));

    auto& param = outputParameters.at(paramName);

    if (param.type != std::type_index(typeid(T)))
      throw std::runtime_error(fmt::format("{}: Output parameter '{}' type mismatch: {} != {}", GetName(), paramName, typeid(T).name(), param.type.name()));

    if (not param.value.has_value())
      throw std::runtime_error(fmt::format("{}: Output parameter '{}' not set", GetName(), paramName));

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

  void SetName(const std::string& name) { microserviceName = name; }

  uintptr_t GetId() const { return reinterpret_cast<uintptr_t>(this); }

  std::unordered_map<std::string, MicroserviceInputParameter>& GetInputParameters() { return inputParameters; }

  std::unordered_map<std::string, MicroserviceOutputParameter>& GetOutputParameters() { return outputParameters; }

  MicroserviceInputParameter& GetStartParameter() { return start; }

  MicroserviceOutputParameter& GetFinishParameter() { return finish; }

  MicroserviceInputParameter& GetInputParameter(const std::string& paramName)
  {
    if (not inputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("{}: Input parameter '{}' not found", GetName(), paramName));

    return inputParameters.at(paramName);
  }

  MicroserviceOutputParameter& GetOutputParameter(const std::string& paramName)
  {
    if (not outputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("{}: Output parameter '{}' not found", GetName(), paramName));

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

  std::optional<MicroserviceInputParameter*> FindInputParameter(uintptr_t parameterId)
  {
    if (start.GetId() == parameterId)
      return &start;

    for (auto& [name, param] : inputParameters)
      if (param.GetId() == parameterId)
        return &param;

    return std::nullopt;
  }

  std::optional<MicroserviceOutputParameter*> FindOutputParameter(uintptr_t parameterId)
  {
    if (finish.GetId() == parameterId)
      return &finish;

    for (auto& [name, param] : outputParameters)
      if (param.GetId() == parameterId)
        return &param;

    return std::nullopt;
  }
};
