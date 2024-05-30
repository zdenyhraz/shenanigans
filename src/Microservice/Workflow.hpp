#pragma once
#include "Microservice.hpp"
#include "Microservice/StartMicroservice.hpp"
#include "Microservice/LoadImageMicroservice.hpp"
#include "Microservice/BlurImageMicroservice.hpp"

class Workflow
{
  struct MicroserviceConnection
  {
    Microservice* outputMicroservice;
    Microservice* inputMicroservice;

    auto operator<=>(const MicroserviceConnection&) const = default;

    uintptr_t GetId() const { return reinterpret_cast<uintptr_t>(this); }
  };

  struct MicroserviceParameterConnection
  {
    MicroserviceOutputParameter* outputParameter;
    MicroserviceInputParameter* inputParameter;

    auto operator<=>(const MicroserviceParameterConnection&) const = default;

    uintptr_t GetId() const { return reinterpret_cast<uintptr_t>(this); }
  };

  std::string name = "Default workflow";
  std::vector<std::unique_ptr<Microservice>> microservices;
  std::vector<MicroserviceConnection> microserviceConnections;
  std::vector<MicroserviceParameterConnection> parameterConnections;

  void Notify(Microservice& microservice)
  {
    auto relevantConnections = std::views::filter(microserviceConnections, [&microservice](auto& conn) { return conn.outputMicroservice == &microservice; });
    for (auto& [outputMicroservice, inputMicroservice] : relevantConnections)
      Execute(*inputMicroservice);
  }

  void Execute(Microservice& microservice)
  try
  {
    microservice.Process(); // do the processing
    Notify(microservice);   // notify connected microservices to start processing
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("Microservice '{}' error: {}", microservice.GetName(), e.what());
  }

public:
  Workflow()
  {
    microservices.push_back(std::make_unique<StartMicroservice>());
    // TODO: load microservices from file
    // TODO: load connections from file
    // TODO: connect the parameters
    // TODO: set workflow name according to file name
  }

  const std::string& GetName() { return name; }
  const std::vector<std::unique_ptr<Microservice>>& GetMicroservices() { return microservices; }
  const std::vector<MicroserviceConnection>& GetMicroserviceConnections() { return microserviceConnections; }
  const std::vector<MicroserviceParameterConnection>& GetMicroserviceParameterConnections() { return parameterConnections; }

  void Initialize()
  {
    LOG_FUNCTION;
    for (const auto& microservice : microservices)
      microservice->Initialize();
  }

  void Run()
  try
  {
    LOG_FUNCTION;

    auto startMicroservice = std::ranges::find_if(microservices, [](const auto& ms) { return dynamic_cast<StartMicroservice*>(ms.get()); });
    if (startMicroservice == microservices.end())
      throw std::runtime_error("Missing start microservice");

    Execute(**startMicroservice);
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("Workflow '{}' error: {}", name, e.what());
  }

  void Connect(MicroserviceOutputParameter& outputParameter, MicroserviceInputParameter& inputParameter)
  {
    inputParameter.value = &outputParameter.value;
    MicroserviceParameterConnection parameterConnection(&outputParameter, &inputParameter);
    if (not std::ranges::any_of(parameterConnections, [&parameterConnection](auto conn) { return conn == parameterConnection; }))
      parameterConnections.push_back(std::move(parameterConnection));
  }

  void Connect(Microservice& outputMicroservice, Microservice& inputMicroservice)
  {
    MicroserviceConnection microserviceConnection(&outputMicroservice, &inputMicroservice);
    if (not std::ranges::any_of(microserviceConnections, [&microserviceConnection](auto conn) { return conn == microserviceConnection; }))
      microserviceConnections.push_back(std::move(microserviceConnection));
  }

  void Connect(Microservice& outputMicroservice, Microservice& inputMicroservice, const std::string& outputParameterName, const std::string& inputParameterName)
  {
    auto& outputParameter = outputMicroservice.GetOutputParameter(outputParameterName);
    auto& inputParameter = inputMicroservice.GetInputParameter(inputParameterName);
    Connect(outputParameter, inputParameter);
  }

  void TestInitialize()
  {
    LOG_FUNCTION;

    microservices.push_back(std::make_unique<BlurImageMicroservice>());
    microservices.push_back(std::make_unique<BlurImageMicroservice>());
    microservices.push_back(std::make_unique<BlurImageMicroservice>());
    microservices.push_back(std::make_unique<BlurImageMicroservice>());
    microservices.push_back(std::make_unique<BlurImageMicroservice>());
    microservices.push_back(std::make_unique<LoadImageMicroservice>());

    Initialize();

    auto& start = *microservices[0];
    auto& blur1 = *microservices[1];
    auto& blur2 = *microservices[2];
    auto& blur3 = *microservices[3];
    auto& blur4 = *microservices[4];
    auto& blur5 = *microservices[5];
    auto& load = *microservices[6];

    Connect(start, load);

    Connect(load, blur1);
    Connect(load, blur1, "image", "image");

    Connect(blur1, blur2);
    Connect(blur1, blur2, "blurred_image", "image");

    Connect(blur2, blur3);
    Connect(blur2, blur3, "blurred_image", "image");

    // from 3 to both 4 & 5 - split

    Connect(blur3, blur4);
    Connect(blur3, blur4, "blurred_image", "image");

    Connect(blur3, blur5);
    Connect(blur3, blur5, "blurred_image", "image");
  }
};
