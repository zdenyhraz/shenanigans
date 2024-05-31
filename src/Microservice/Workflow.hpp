#pragma once
#include "Microservice.hpp"
#include "Microservice/StartMicroservice.hpp"
#include "Microservice/LoadImageMicroservice.hpp"
#include "Microservice/BlurImageMicroservice.hpp"

class Workflow
{
  struct MicroserviceConnection
  {
    Microservice* outputMicroservice = nullptr;
    Microservice* inputMicroservice = nullptr;
    MicroserviceOutputParameter* outputParameter = nullptr;
    MicroserviceInputParameter* inputParameter = nullptr;

    auto operator<=>(const MicroserviceConnection&) const = default;

    uintptr_t GetId() const { return reinterpret_cast<uintptr_t>(this); }
  };

  std::string name = "Default workflow";
  std::vector<std::unique_ptr<Microservice>> microservices;
  std::unordered_map<Microservice*, std::vector<MicroserviceConnection>> connections;

  void ExecuteMicroservice(Microservice& microservice)
  try
  {
    // 1) do the processing
    microservice.Process();

    // 2) notify connected microservices to start processing
    if (not connections.contains(&microservice))
      return;

    auto relevantConnections = std::views::filter(connections.at(&microservice),
        [&microservice](MicroserviceConnection& conn) { return conn.outputMicroservice == &microservice and conn.outputParameter == &microservice.GetFinishParameter(); });
    LOG_DEBUG("Microservice '{}' has {} output connections:", microservice.GetName(), std::ranges::distance(relevantConnections));
    for (auto& connection : relevantConnections)
      LOG_DEBUG("'{}'", connection.inputMicroservice->GetName());

    for (auto& connection : relevantConnections)
      ExecuteMicroservice(*connection.inputMicroservice);
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("Microservice '{}' error: {}", microservice.GetName(), e.what());
  }

public:
  Workflow() { microservices.push_back(std::make_unique<StartMicroservice>()); }

  Workflow(const std::filesystem::path& path) : Workflow() { Load(path); }

  void Load(const std::filesystem::path& path)
  {
    // TODO: load microservices from file
    // TODO: load connections from file
    // TODO: connect the parameters
    // TODO: set workflow name according to file name
    // TODO: set node locations from file
  }

  void Save(const std::filesystem::path& path) const
  {
    // TODO: serialize microservices to json
    // TODO: serialize connections to json
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
      throw std::runtime_error(fmt::format("Workflow '{}' is missing a start microservice", GetName()));

    ExecuteMicroservice(**startMicroservice);
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("Workflow '{}' error: {}", GetName(), e.what());
  }

  void Connect(MicroserviceConnection&& connection)
  {
    if (connection.outputParameter->type != connection.inputParameter->type)
      return LOG_WARNING("Connection {}:{} -> {}:{} type mismatch: {} != {}", connection.outputMicroservice->GetName(), connection.outputParameter->GetName(),
          connection.inputMicroservice->GetName(), connection.inputParameter->GetName(), connection.inputParameter->type.name(), connection.outputParameter->type.name());
    if (connection.outputMicroservice == connection.inputMicroservice)
      return LOG_WARNING("Connection {}:{} -> {}:{}: cannot connect microservice to itself ", connection.outputMicroservice->GetName(), connection.outputParameter->GetName(),
          connection.inputMicroservice->GetName(), connection.inputParameter->GetName());
    if (connections.contains(connection.outputMicroservice))
      if (std::ranges::any_of(connections.at(connection.outputMicroservice), [&connection](auto& conn) { return conn == connection; }))
        return LOG_WARNING("Ignoring duplicate connection {}:{} -> {}:{}", connection.outputMicroservice->GetName(), connection.outputParameter->GetName(),
            connection.inputMicroservice->GetName(), connection.inputParameter->GetName());

    LOG_DEBUG("Connected {}:{} -> {}:{}", connection.outputMicroservice->GetName(), connection.outputParameter->GetName(), connection.inputMicroservice->GetName(),
        connection.inputParameter->GetName());

    connection.inputParameter->value = &connection.outputParameter->value;
    connections[connection.outputMicroservice].push_back(std::move(connection));
  }

  void Connect(Microservice& outputMicroservice, Microservice& inputMicroservice, const std::string& outputParameterName, const std::string& inputParameterName)
  {
    auto& outputParameter = outputMicroservice.GetOutputParameter(outputParameterName);
    auto& inputParameter = inputMicroservice.GetInputParameter(inputParameterName);
    MicroserviceConnection connection(&outputMicroservice, &inputMicroservice, &outputParameter, &inputParameter);
    Connect(std::move(connection));
  }

  void Connect(Microservice& outputMicroservice, Microservice& inputMicroservice)
  {
    MicroserviceConnection connection(&outputMicroservice, &inputMicroservice, &outputMicroservice.GetFinishParameter(), &inputMicroservice.GetStartParameter());
    Connect(std::move(connection));
  }

  void Connect(uintptr_t startId, uintptr_t endId)
  {
    MicroserviceConnection connection;
    for (const auto& microservice : microservices)
    {
      if (auto param = microservice->FindInputParameter(startId))
      {
        connection.inputMicroservice = microservice.get();
        connection.inputParameter = param.value();
      }
      if (auto param = microservice->FindInputParameter(endId))
      {
        connection.inputMicroservice = microservice.get();
        connection.inputParameter = param.value();
      }
      if (auto param = microservice->FindOutputParameter(startId))
      {
        connection.outputMicroservice = microservice.get();
        connection.outputParameter = param.value();
      }
      if (auto param = microservice->FindOutputParameter(endId))
      {
        connection.outputMicroservice = microservice.get();
        connection.outputParameter = param.value();
      }

      if (connection.outputMicroservice and connection.inputMicroservice)
        break;
    }

    if (not connection.outputMicroservice or not connection.inputMicroservice)
      return LOG_WARNING("Please connect outputs to inputs");

    Connect(std::move(connection));
  }

  void Disconnect(uintptr_t connectionId)
  {
    for (auto& [ms, connectionvec] : connections)
    {
      const auto size = connectionvec.size();
      auto conn = std::ranges::remove_if(connectionvec, [connectionId](const auto& conn) { return conn.GetId() == connectionId; });
      connectionvec.erase(conn.begin(), connectionvec.end());
      if (connectionvec.size() != size)
        return LOG_DEBUG("Disconnected connection {}", connectionId);
    }
    LOG_WARNING("Ignoring disconnect of connection {}", connectionId);
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
