#pragma once

class Microservice
{
  friend class Workflow;

  struct MicroserviceParameter
  {
    std::type_index type = std::type_index(typeid(void));
    std::any value;

    auto operator<=>(const MicroserviceParameter&) const = default;

    void Reset()
    {
      type = std::type_index(typeid(void));
      value.reset();
    }

    template <typename T>
    bool CheckType()
    {
      return std::type_index(typeid(T)) == type;
    }

    template <typename T>
    static MicroserviceParameter Get(T&& _value)
    {
      return MicroserviceParameter(std::type_index(typeid(T)), std::forward<T>(_value));
    }
  };

  struct MicroserviceConnection
  {
    Microservice* outputMicroservice;
    Microservice* inputMicroservice;

    auto operator<=>(const MicroserviceConnection&) const = default;
  };

  struct MicroserviceParameterConnection
  {
    Microservice* outputMicroservice;
    Microservice* inputMicroservice;
    std::string outputParameterName;
    std::string inputParameterName;

    auto operator<=>(const MicroserviceParameterConnection&) const = default;
  };

  std::string microserviceName;
  usize microserviceId;
  std::unordered_map<std::string, MicroserviceParameter> inputParameters;
  std::unordered_map<std::string, MicroserviceParameter> outputParameters;

  std::vector<MicroserviceParameterConnection> inputParameterConnections;
  std::vector<MicroserviceParameterConnection> outputParameterConnections;

  std::vector<std::shared_ptr<Microservice>> outputMicroservices;

protected:
  virtual void DefineInputParameters() = 0;
  virtual void DefineOutputParameters() = 0;
  virtual void Process() = 0;

  template <typename T>
  void DefineInputParameter(const std::string& paramName)
  {
    inputParameters[paramName] = MicroserviceParameter(std::type_index(typeid(T)));
  }

  template <typename T>
  void DefineOutputParameter(const std::string& paramName)
  {
    outputParameters[paramName] = MicroserviceParameter(std::type_index(typeid(T)));
  }

  void SetInputParameter(const std::string& paramName, const MicroserviceParameter& value)
  {
    if (not inputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("Input parameter '{}' not found", paramName));

    auto& param = inputParameters.at(paramName);

    param = value;
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

  MicroserviceParameter& GetOutputParameter(const std::string& paramName)
  {
    if (not outputParameters.contains(paramName))
      throw std::runtime_error(fmt::format("Output parameter '{}' not found", paramName));

    auto& param = outputParameters.at(paramName);

    if (not param.value.has_value())
      throw std::runtime_error(fmt::format("Output parameter '{}' not set", paramName));

    return param;
  }

  void PropagateOutputs()
  {
    for (const auto& connection : outputParameterConnections)
    {
      LOG_DEBUG("{}:{} -> {}:{}", GetName(), connection.outputParameterName, connection.inputMicroservice->GetName(), connection.inputParameterName);
      connection.inputMicroservice->SetInputParameter(connection.inputParameterName, GetOutputParameter(connection.outputParameterName));
    }
  }

  void Notify()
  {
    for (auto& microservice : outputMicroservices)
    {
      LOG_DEBUG("{} -> {}", GetName(), microservice->GetName());
      microservice->Execute();
    }
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

    static usize idx = 0;
    microserviceId = idx += 100; // for unique input/output pin ids
    microserviceName = fmt::format("{}{}", typeName, microserviceId);
  }

public:
  virtual ~Microservice() {}

  const std::string& GetName() { return microserviceName; }

  usize GetId() { return microserviceId; }

  const std::unordered_map<std::string, MicroserviceParameter>& GetInputParameters() { return inputParameters; }

  const std::unordered_map<std::string, MicroserviceParameter>& GetOutputParameters() { return outputParameters; }

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

  static void Connect(std::shared_ptr<Microservice> outputMicroservice, std::shared_ptr<Microservice> inputMicroservice, const std::string& outputParameterName,
      const std::string& inputParameterName) // connect output / input parameters
  {
    if (not outputMicroservice->outputParameters.contains(outputParameterName))
      throw std::runtime_error(fmt::format("Cannot connect microservice '{}' parameter '{}' to microservice '{}' parameter '{}' - output parameter not found",
          outputMicroservice->GetName(), outputParameterName, inputMicroservice->GetName(), inputParameterName));
    if (not inputMicroservice->inputParameters.contains(inputParameterName))
      throw std::runtime_error(fmt::format("Cannot connect microservice '{}' parameter '{}' to microservice '{}' parameter '{}' - input parameter not found",
          outputMicroservice->GetName(), outputParameterName, inputMicroservice->GetName(), inputParameterName));

    MicroserviceParameterConnection connection(outputMicroservice.get(), inputMicroservice.get(), outputParameterName, inputParameterName);
    if (not std::ranges::any_of(outputMicroservice->outputParameterConnections, [&connection](const auto& conn) { return conn == connection; }))
      outputMicroservice->outputParameterConnections.push_back(connection);
    else
      LOG_WARNING(
          "Ignoring duplicate output parameter connection: {}:{} -> {}:{}", outputMicroservice->GetName(), outputParameterName, inputMicroservice->GetName(), inputParameterName);
    if (not std::ranges::any_of(inputMicroservice->inputParameterConnections, [&connection](const auto& conn) { return conn == connection; }))
      inputMicroservice->inputParameterConnections.push_back(connection);
    else
      LOG_WARNING(
          "Ignoring duplicate input parameter connection: {}:{} -> {}:{}", outputMicroservice->GetName(), outputParameterName, inputMicroservice->GetName(), inputParameterName);
  }

  static void Connect(std::shared_ptr<Microservice> outputMicroservice, std::shared_ptr<Microservice> inputMicroservice) // main execution flow: notify when done processing
  {
    if (not std::ranges::any_of(outputMicroservice->outputMicroservices, [&inputMicroservice](const auto& ms) { return ms.get() == inputMicroservice.get(); }))
      outputMicroservice->outputMicroservices.push_back(inputMicroservice);
    else
      LOG_WARNING("Ignoring duplicate microservice connection: {} -> {}", outputMicroservice->GetName(), inputMicroservice->GetName());
  }

  bool HasInputConnection() const { return inputParameterConnections.size() > 0; }

  void Execute()
  try
  {
    Process();          // do the processing
    PropagateOutputs(); // propagate outputs to connected inputs
    Notify();           // notify connected microservices to start processing
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("Microservice '{}' error: {}", microserviceName, e.what());
  }
};
