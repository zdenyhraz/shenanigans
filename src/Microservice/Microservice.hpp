#pragma once

class Microservice
{
public:
  struct FlowParameter
  {
  };

  struct InputParameter
  {
    const std::type_info& type;
    std::string name;
    std::any* value = nullptr; // pointer to avoid data duplication

    InputParameter(const std::type_info& _type, const std::string& _name) : type(_type), name(_name), value(nullptr) {}

    uintptr_t GetId() const { return reinterpret_cast<uintptr_t>(this); }
    const std::string& GetName() const { return name; }
    void Reset() { value = nullptr; }
  };

  struct OutputParameter
  {
    const std::type_info& type;
    std::string name;
    std::any value;

    OutputParameter(const std::type_info& _type, const std::string& _name) : type(_type), name(_name) {}

    uintptr_t GetId() const { return reinterpret_cast<uintptr_t>(this); }
    const std::string& GetName() const { return name; }
    void Reset() { value.reset(); }
  };

  struct Connection
  {
    Microservice* outputMicroservice = nullptr;
    Microservice* inputMicroservice = nullptr;
    OutputParameter* outputParameter = nullptr;
    InputParameter* inputParameter = nullptr;

    auto operator<=>(const Connection&) const = default;

    uintptr_t GetId() const { return reinterpret_cast<uintptr_t>(this); }

    std::string GetString() const
    {
      return fmt::format("{}:{} -> {}:{}", outputMicroservice->GetName(), outputParameter->GetName(), inputMicroservice->GetName(), inputParameter->GetName());
    }
  };

private:
  std::string microserviceName;
  InputParameter start = InputParameter{typeid(FlowParameter), ""};
  OutputParameter finish = OutputParameter{typeid(FlowParameter), ""};
  std::unordered_map<std::string, InputParameter> inputParameters;
  std::unordered_map<std::string, OutputParameter> outputParameters;
  std::vector<Connection> inputConnections;
  std::vector<Connection> outputConnections;

protected:
  virtual void DefineInputParameters() = 0;
  virtual void DefineOutputParameters() = 0;

  template <typename T>
  void DefineInputParameter(const std::string& paramName)
  {
    inputParameters.emplace(paramName, InputParameter{typeid(T), paramName});
  }

  template <typename T>
  void DefineOutputParameter(const std::string& paramName)
  {
    outputParameters.emplace(paramName, OutputParameter{typeid(T), paramName});
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

  std::unordered_map<std::string, InputParameter>& GetInputParameters() { return inputParameters; }

  std::unordered_map<std::string, OutputParameter>& GetOutputParameters() { return outputParameters; }

  const std::vector<Connection>& GetInputConnections() { return inputConnections; }

  const std::vector<Connection>& GetOutputConnections() { return outputConnections; }

  InputParameter& GetStartParameter() { return start; }

  OutputParameter& GetFinishParameter() { return finish; }

  InputParameter& GetInputParameter(const std::string& paramName)
  {
    if (not inputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("{}: Input parameter '{}' not found", GetName(), paramName));

    return inputParameters.at(paramName);
  }

  OutputParameter& GetOutputParameter(const std::string& paramName)
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

  std::optional<InputParameter*> FindInputParameter(uintptr_t parameterId)
  {
    if (start.GetId() == parameterId)
      return &start;

    for (auto& [name, param] : inputParameters)
      if (param.GetId() == parameterId)
        return &param;

    return std::nullopt;
  }

  std::optional<OutputParameter*> FindOutputParameter(uintptr_t parameterId)
  {
    if (finish.GetId() == parameterId)
      return &finish;

    for (auto& [name, param] : outputParameters)
      if (param.GetId() == parameterId)
        return &param;

    return std::nullopt;
  }

  void AddInputConnection(const Connection& connection) { inputConnections.push_back(connection); }

  void AddOutputConnection(const Connection& connection) { outputConnections.push_back(connection); }

  void RemoveInputConnection(const Connection& connection) { inputConnections.erase(std::ranges::remove(inputConnections, connection).begin(), inputConnections.end()); }

  void RemoveOutputConnection(const Connection& connection) { outputConnections.erase(std::ranges::remove(outputConnections, connection).begin(), outputConnections.end()); }
};
