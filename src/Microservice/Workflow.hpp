#pragma once
#include "Microservice.hpp"
#include "Microservice/BlurImageMicroservice.hpp"

class Workflow
{
  std::string name = "Default workflow";
  std::vector<std::shared_ptr<Microservice>> microservices;
  std::vector<Microservice::MicroserviceConnection> microserviceConnections;
  std::vector<Microservice::MicroserviceParameterConnection> parameterConnections;

public:
  const std::string& GetName() { return name; }
  const std::vector<std::shared_ptr<Microservice>>& GetMicroservices() { return microservices; }
  const std::vector<Microservice::MicroserviceConnection>& GetMicroserviceConnections() { return microserviceConnections; }
  const std::vector<Microservice::MicroserviceParameterConnection>& GetParameterConnections() { return parameterConnections; }

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
    // TODO: only call on ms with no imputs, or execute back-to-front?
    auto startMicroservices = microservices | std::views::filter([](const auto& ms) { return not ms->HasInputConnection(); });
    if (startMicroservices.empty())
      throw std::runtime_error("Could not determine starting microservices");
    for (const auto& microservice : startMicroservices)
    {
      LOG_DEBUG("Executing starting microservice {}", microservice->GetName());
      microservice->Execute();
    }
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("Workflow '{}' error: {}", name, e.what());
  }

  void Connect(std::shared_ptr<Microservice> outputMicroservice, std::shared_ptr<Microservice> inputMicroservice, const std::string& outputParameterName,
      const std::string& inputParameterName)
  {
    Microservice::Connect(outputMicroservice, inputMicroservice, outputParameterName, inputParameterName);
    parameterConnections.emplace_back(outputMicroservice.get(), inputMicroservice.get(), outputParameterName, inputParameterName);
  }

  void Connect(std::shared_ptr<Microservice> outputMicroservice, std::shared_ptr<Microservice> inputMicroservice)
  {
    Microservice::Connect(outputMicroservice, inputMicroservice);
    microserviceConnections.emplace_back(outputMicroservice.get(), inputMicroservice.get());
  }

  void TestInitialize()
  {
    LOG_FUNCTION;

    microservices.push_back(std::make_shared<BlurImageMicroservice>());
    microservices.push_back(std::make_shared<BlurImageMicroservice>());
    microservices.push_back(std::make_shared<BlurImageMicroservice>());
    microservices.push_back(std::make_shared<BlurImageMicroservice>());
    microservices.push_back(std::make_shared<BlurImageMicroservice>());
    Initialize();

    auto blur1 = microservices[0];
    auto blur2 = microservices[1];
    auto blur3 = microservices[2];
    auto blur4 = microservices[3];
    auto blur5 = microservices[4];

    auto image = cv::imread(GetProjectDirectoryPath("data/ml/object_detection/datasets/cats/cats2.jpg").string());
    blur1->SetInputParameter("image", Microservice::MicroserviceParameter::Get(std::move(image)));

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
