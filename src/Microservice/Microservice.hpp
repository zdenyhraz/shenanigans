#pragma once

class Microservice
{
public:
  class Exception : public std::exception
  {
  public:
    template <typename... Args>
    Exception(const Microservice* microservice, std::string_view fmt, Args&&... args)
    {
      message = fmt::format("Microservice '{}' error: {}", microservice->GetName(), fmt::vformat(fmt, fmt::make_format_args(args...)));
    }

    virtual const char* what() const noexcept { return message.c_str(); }

  private:
    std::string message;
  };

  template <typename... Args>
  Exception MicroserviceException(std::string_view fmt, Args&&... args)
  {
    return Exception(this, fmt, std::forward<Args>(args)...);
  }

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

  struct Parameter
  {
    const std::type_info& type;
    std::string name;
    std::any value;

    template <typename T>
    Parameter(const std::type_info& _type, const std::string& _name, const T& _value) : type(_type), name(_name), value(_value)
    {
    }
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
  InputParameter flowInput = InputParameter{typeid(FlowParameter), ""};
  OutputParameter flowOutput = OutputParameter{typeid(FlowParameter), ""};
  std::unordered_map<std::string, InputParameter> inputParameters;
  std::unordered_map<std::string, OutputParameter> outputParameters;
  std::unordered_map<std::string, Parameter> parameters;
  std::vector<Connection> inputConnections;
  std::vector<Connection> outputConnections;

protected:
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
  void DefineParameter(const std::string& paramName, const T& defaultValue)
  {
    parameters.emplace(paramName, Parameter{typeid(T), paramName, defaultValue});
  }

  template <typename T>
  void SetInputParameter(const std::string& paramName, T& value)
  {
    if (not inputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("{}: Input parameter '{}' not defined", GetName(), paramName));

    auto& param = inputParameters.at(paramName);

    if (param.type != std::type_index(typeid(T)))
      throw std::runtime_error(fmt::format("{}: Input parameter '{}' type mismatch: {} != {}", GetName(), paramName, typeid(T).name(), param.type.name()));

    param.value = &value;
  }

  template <typename T>
  T& GetInputParameter(const std::string& paramName)
  {
    if (not inputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("{}: Input parameter '{}' not defined", GetName(), paramName));

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
      throw std::runtime_error(fmt::format("{}: Output parameter '{}' not defined", GetName(), paramName));

    auto& param = outputParameters.at(paramName);

    if (param.type != std::type_index(typeid(T)))
      throw std::runtime_error(fmt::format("{}: Output parameter '{}' type mismatch: {} != {}", GetName(), paramName, typeid(T).name(), param.type.name()));

    param.value = value;
  }

  template <typename T>
  T& GetOutputParameter(const std::string& paramName)
  {
    if (not outputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("{}: Output parameter '{}' not defined", GetName(), paramName));

    auto& param = outputParameters.at(paramName);

    if (param.type != std::type_index(typeid(T)))
      throw std::runtime_error(fmt::format("{}: Output parameter '{}' type mismatch: {} != {}", GetName(), paramName, typeid(T).name(), param.type.name()));

    if (not param.value.has_value())
      throw std::runtime_error(fmt::format("{}: Output parameter '{}' not set", GetName(), paramName));

    return std::any_cast<T&>(param.value);
  }

  template <typename T>
  T& GetParameter(const std::string& paramName)
  {
    if (not parameters.contains(paramName))
      throw std::runtime_error(fmt::format("{}: Parameter '{}' not defined", GetName(), paramName));

    auto& param = parameters.at(paramName);

    if (param.type != std::type_index(typeid(T)))
      throw std::runtime_error(fmt::format("{}: Parameter '{}' type mismatch: {} != {}", GetName(), paramName, typeid(T).name(), param.type.name()));

    if (not param.value.has_value())
      throw std::runtime_error(fmt::format("{}: Parameter '{}' not set", GetName(), paramName));

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

  const std::string& GetName() const { return microserviceName; }

  void SetName(const std::string& name) { microserviceName = name; }

  uintptr_t GetId() const { return reinterpret_cast<uintptr_t>(this); }

  std::unordered_map<std::string, InputParameter>& GetInputParameters() { return inputParameters; }

  std::unordered_map<std::string, OutputParameter>& GetOutputParameters() { return outputParameters; }

  std::unordered_map<std::string, Parameter>& GetParameters() { return parameters; }

  const std::vector<Connection>& GetInputConnections() { return inputConnections; }

  const std::vector<Connection>& GetOutputConnections() { return outputConnections; }

  InputParameter& GetFlowInputParameter() { return flowInput; }

  OutputParameter& GetFlowOutputParameter() { return flowOutput; }

  void AddInputConnection(const Connection& connection) { inputConnections.push_back(connection); }

  void AddOutputConnection(const Connection& connection) { outputConnections.push_back(connection); }

  void RemoveInputConnection(const Connection& connection) { inputConnections.erase(std::ranges::remove(inputConnections, connection).begin(), inputConnections.end()); }

  void RemoveOutputConnection(const Connection& connection) { outputConnections.erase(std::ranges::remove(outputConnections, connection).begin(), outputConnections.end()); }

  bool IsInputConnected(const InputParameter& param)
  {
    return std::ranges::any_of(inputConnections, [&param](const auto& conn) { return conn.inputParameter == &param; });
  }

  bool IsOutputConnected(const OutputParameter& param)
  {
    return std::ranges::any_of(outputConnections, [&param](const auto& conn) { return conn.outputParameter == &param; });
  }

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

  template <typename T>
  void SetParameter(const std::string& paramName, const T& value)
  {
    if (not parameters.contains(paramName))
      throw std::runtime_error(fmt::format("{}: Parameter '{}' not defined", GetName(), paramName));

    auto& param = parameters.at(paramName);

    if (param.type != std::type_index(typeid(T)))
      throw std::runtime_error(fmt::format("{}: Parameter '{}' type mismatch: {} != {}", GetName(), paramName, typeid(T).name(), param.type.name()));

    param.value = value;
  }

  void Reset()
  {
    // call this on repeated workflows to recompute output parameters
    for (auto& [name, param] : outputParameters)
      param.Reset();
  }

  std::optional<InputParameter*> FindInputParameter(uintptr_t parameterId)
  {
    if (flowInput.GetId() == parameterId)
      return &flowInput;

    for (auto& [name, param] : inputParameters)
      if (param.GetId() == parameterId)
        return &param;

    return std::nullopt;
  }

  std::optional<OutputParameter*> FindOutputParameter(uintptr_t parameterId)
  {
    if (flowOutput.GetId() == parameterId)
      return &flowOutput;

    for (auto& [name, param] : outputParameters)
      if (param.GetId() == parameterId)
        return &param;

    return std::nullopt;
  }
};
