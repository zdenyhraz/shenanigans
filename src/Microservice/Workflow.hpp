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
    MicroserviceOutputParameter* outputParameter;
    MicroserviceInputParameter* inputParameter;

    auto operator<=>(const MicroserviceConnection&) const = default;

    uintptr_t GetId() const { return reinterpret_cast<uintptr_t>(this); }
  };

  std::string name = "Default workflow";
  std::vector<std::unique_ptr<Microservice>> microservices;
  std::unordered_map<Microservice*, std::vector<MicroserviceConnection>> connections;

  void Execute(Microservice& microservice)
  try
  {
    // do the processing
    microservice.Process();

    // notify connected microservices to start processing
    if (not connections.contains(&microservice))
      return;

    auto relevantConnections = std::views::filter(connections.at(&microservice),
        [&microservice](MicroserviceConnection& conn) { return conn.outputMicroservice == &microservice and conn.outputParameter == &microservice.finish; });
    LOG_DEBUG("Microservice '{}' has {} output connections:", microservice.GetName(), std::ranges::distance(relevantConnections));
    for (auto& connection : relevantConnections)
      LOG_DEBUG("'{}'", connection.inputMicroservice->GetName());
    for (auto& connection : relevantConnections)
      Execute(*connection.inputMicroservice);
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
  const std::unordered_map<Microservice*, std::vector<MicroserviceConnection>>& GetConnections() { return connections; }

  void Initialize()
  {
    for (const auto& microservice : microservices)
      microservice->Initialize();
  }

  void Run()
  try
  {
    LOG_DEBUG("Running workflow '{}'", GetName());
    auto startMicroservice = std::ranges::find_if(microservices, [](const auto& ms) { return dynamic_cast<StartMicroservice*>(ms.get()); });
    if (startMicroservice == microservices.end())
      throw std::runtime_error("Missing start microservice");

    Execute(**startMicroservice);
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("Workflow '{}' error: {}", GetName(), e.what());
  }

  void Connect(Microservice& outputMicroservice, Microservice& inputMicroservice, const std::string& outputParameterName, const std::string& inputParameterName)
  {
    auto& outputParameter = outputMicroservice.GetOutputParameter(outputParameterName);
    auto& inputParameter = inputMicroservice.GetInputParameter(inputParameterName);
    MicroserviceConnection connection(&outputMicroservice, &inputMicroservice, &outputParameter, &inputParameter);
    if (connections.contains(&outputMicroservice))
      if (std::ranges::any_of(connections.at(&outputMicroservice), [&connection](auto& conn) { return conn == connection; }))
        return LOG_WARNING("Ignoring duplicate connection {}:{} -> {}:{}", outputMicroservice.GetName(), outputParameterName, inputMicroservice.GetName(), inputParameterName);

    connections[&outputMicroservice].push_back(std::move(connection));
  }

  void Connect(Microservice& outputMicroservice, Microservice& inputMicroservice)
  {
    MicroserviceConnection connection(&outputMicroservice, &inputMicroservice, &outputMicroservice.finish, &inputMicroservice.start);
    if (connections.contains(&outputMicroservice))
      if (std::ranges::any_of(connections.at(&outputMicroservice), [&connection](auto& conn) { return conn == connection; }))
        return LOG_WARNING("Ignoring duplicate connection {} -> {}", outputMicroservice.GetName(), inputMicroservice.GetName());

    connections[&outputMicroservice].push_back(std::move(connection));
  }

  void Connect(uintptr_t outputId, uintptr_t inputId)
  {
    MicroserviceConnection connection;
    bool outputFound = false;
    bool inputFound = false;
    for (const auto& microservice : microservices)
    {
      if (microservice->finish.GetId() == outputId)
      {
        connection.outputMicroservice = microservice.get();
        connection.outputParameter = &microservice->finish;
        outputFound = true;
      }
      else
      {
        for (auto& [name, param] : microservice->GetOutputParameters())
        {
          if (param.GetId() == outputId)
          {
            connection.outputMicroservice = microservice.get();
            connection.outputParameter = &param;
            outputFound = true;
            break;
          }
        }
      }

      if (microservice->start.GetId() == inputId)
      {
        connection.inputMicroservice = microservice.get();
        connection.inputParameter = &microservice->start;
        inputFound = true;
      }
      else
      {
        for (auto& [name, param] : microservice->GetInputParameters())
        {
          if (param.GetId() == inputId)
          {
            connection.inputMicroservice = microservice.get();
            connection.inputParameter = &param;
            inputFound = true;
            break;
          }
        }
      }
      if (outputFound and inputFound)
        break;
    }

    if (not connection.outputMicroservice)
      throw std::runtime_error(fmt::format("Failed to find output parameter for id {}", outputId));

    if (not connection.inputMicroservice)
      throw std::runtime_error(fmt::format("Failed to find input parameter for id {}", inputId));

    if (connections.contains(connection.outputMicroservice))
      if (std::ranges::any_of(connections.at(connection.outputMicroservice), [&connection](auto& conn) { return conn == connection; }))
        return LOG_WARNING("Ignoring duplicate connection {}:{} -> {}:{}", connection.outputMicroservice->GetName(), connection.outputParameter->GetId(),
            connection.inputMicroservice->GetName(), connection.inputParameter->GetId());

    connections[connection.outputMicroservice].push_back(std::move(connection));
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
    Connect(blur1, blur2, "blurred image", "image");

    Connect(blur2, blur3);
    Connect(blur2, blur3, "blurred image", "image");

    // from 3 to both 4 & 5 - split

    Connect(blur3, blur4);
    Connect(blur3, blur4, "blurred image", "image");

    Connect(blur3, blur5);
    Connect(blur3, blur5, "blurred image", "image");
  }
};
