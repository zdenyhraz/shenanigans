#pragma once
#include "Microservice.hpp"
#include "Microservices/StartMicroservice.hpp"

class Workflow
{
  std::string name = "Default workflow";
  std::vector<std::unique_ptr<Microservice>> microservices;
  std::vector<Microservice::Connection> connections;

  void FetchInputParameters(Microservice& microservice)
  {
    auto relevantConnections = std::views::filter(microservice.GetInputConnections(), [](const auto& conn)
        { return conn.inputParameter->value and not conn.inputParameter->value->has_value() and conn.inputParameter != &conn.inputMicroservice->GetFlowInputParameter(); });

    if (std::ranges::distance(relevantConnections) == 0)
      return;

    LOG_DEBUG("Microservice '{}' has {} input parameter dependencies:", microservice.GetName(), std::ranges::distance(relevantConnections));
    for (const auto& connection : relevantConnections)
      LOG_DEBUG("   {}", connection.GetString());

    for (auto& connection : relevantConnections)
      ExecuteMicroservice(*connection.outputMicroservice);
  }

  void Process(Microservice& microservice)
  {
    LOG_DEBUG(">>> {} processing", microservice.GetName());
    microservice.Process();
  }

  void PropagateFlow(Microservice& microservice)
  {
    auto relevantConnections =
        std::views::filter(microservice.GetOutputConnections(), [](const auto& conn) { return conn.outputParameter == &conn.outputMicroservice->GetFlowOutputParameter(); });

    if (std::ranges::distance(relevantConnections) == 0)
      return;

    LOG_DEBUG("Microservice '{}' has {} output connections:", microservice.GetName(), std::ranges::distance(relevantConnections));
    for (const auto& connection : relevantConnections)
      LOG_DEBUG("   {}", connection.GetString());

    for (auto& connection : relevantConnections)
      ExecuteMicroservice(*connection.inputMicroservice);
  }

  void ExecuteMicroservice(Microservice& microservice)
  {
    FetchInputParameters(microservice); // recusively fetch all input parameters
    Process(microservice);              // do the processing
    PropagateFlow(microservice);        // notify connected microservices to start processing
  }

public:
  Workflow() { microservices.push_back(std::make_unique<StartMicroservice>()); }

  Workflow(const std::filesystem::path& path) { Load(path); }

  void Load(const std::filesystem::path& path)
  {
    // TODO: load microservices from file (with parameters!)
    // TODO: load connections from file
    // TODO: connect the parameters
    // TODO: set workflow name according to file name
    // TODO: set node locations from file
  }

  void Save(const std::filesystem::path& path) const
  {
    // TODO: serialize microservices to json (with parameters!)
    // TODO: serialize connections to json
  }

  const std::string& GetName() { return name; }
  const std::vector<std::unique_ptr<Microservice>>& GetMicroservices() { return microservices; }
  const std::vector<Microservice::Connection>& GetConnections() { return connections; }

  template <typename T>
  void AddMicroservice()
  {
    microservices.push_back(std::make_unique<T>());
  }

  void Run()
  try
  {
    LOG_DEBUG("Running workflow '{}'", GetName());

    if (microservices.empty())
      throw std::runtime_error(fmt::format("Workflow '{}' is missing a start microservice", GetName()));

    if (microservices.size() == 1)
      return LOG_WARNING("Workflow '{}' does not contain any microservices", GetName());

    ExecuteMicroservice(*microservices.front());
    Reset();
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("Workflow '{}' error: {}", GetName(), e.what());
  }

  void Reset()
  {
    for (auto& microservice : microservices)
      microservice->Reset();
  }

  void Connect(Microservice::Connection&& connection)
  {
    if (not connection.outputMicroservice or not connection.inputMicroservice or not connection.outputParameter or not connection.inputParameter)
      return LOG_WARNING("Ignoring invalid connection");

    if (std::ranges::any_of(connections, [&connection](const auto& conn) { return conn == connection; }))
      return LOG_WARNING("Ignoring duplicate connection {}", connection.GetString());

    if (connection.outputParameter->type != connection.inputParameter->type)
      return LOG_WARNING("Connection {} type mismatch: {} != {}", connection.GetString(), connection.inputParameter->type.name(), connection.outputParameter->type.name());

    if (connection.inputParameter->type != typeid(Microservice::FlowParameter))
      if (std::ranges::any_of(connections, [&connection](const auto& conn) { return conn.inputParameter == connection.inputParameter; }))
        return LOG_WARNING("Connection to {}:{} already exists", connection.inputMicroservice->GetName(), connection.inputParameter->GetName());

    if (connection.outputMicroservice == connection.inputMicroservice)
      return LOG_WARNING("Connection {}: cannot connect microservice to itself ", connection.GetString());

    LOG_TRACE("Connected {}", connection.GetString());

    connection.inputParameter->value = &connection.outputParameter->value;
    connection.outputMicroservice->AddOutputConnection(connection);
    connection.inputMicroservice->AddInputConnection(connection);
    connections.push_back(std::move(connection));
  }

  void Connect(Microservice& outputMicroservice, Microservice& inputMicroservice, const std::string& outputParameterName, const std::string& inputParameterName)
  {
    auto& outputParameter = outputMicroservice.GetOutputParameter(outputParameterName);
    auto& inputParameter = inputMicroservice.GetInputParameter(inputParameterName);
    Microservice::Connection connection(&outputMicroservice, &inputMicroservice, &outputParameter, &inputParameter);
    Connect(std::move(connection));
  }

  void Connect(Microservice& outputMicroservice, Microservice& inputMicroservice)
  {
    Microservice::Connection connection(&outputMicroservice, &inputMicroservice, &outputMicroservice.GetFlowOutputParameter(), &inputMicroservice.GetFlowInputParameter());
    Connect(std::move(connection));
  }

  void Disconnect(uintptr_t connectionId)
  {
    auto conn = std::ranges::find_if(connections, [connectionId](const auto& conn) { return conn.GetId() == connectionId; });
    if (conn == connections.end())
      return LOG_WARNING("Ignoring disconnect of connection {}: connection not found", connectionId);

    const auto connection = *conn;
    connections.erase(std::ranges::remove(connections, connection).begin(), connections.end());
    connection.inputParameter->value = nullptr;
    connection.outputMicroservice->RemoveOutputConnection(connection);
    connection.inputMicroservice->RemoveInputConnection(connection);
    return LOG_TRACE("Disconnected connection {}", connection.GetString());
  }
};
