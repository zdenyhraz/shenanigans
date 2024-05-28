#pragma once
#include "Microservice.hpp"
#include "Microservice/BlurImageMicroservice.hpp"

class Workflow
{
  std::string name = "Default workflow";
  std::vector<std::shared_ptr<Microservice>> microservices;

public:
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

  void Test()
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

    Microservice::Connect(blur1, blur2);
    Microservice::Connect(blur1, blur2, "blurred_image", "image");

    Microservice::Connect(blur2, blur3);
    Microservice::Connect(blur2, blur3, "blurred_image", "image");

    // from 3 to both 4 & 5 - split

    Microservice::Connect(blur3, blur4);
    Microservice::Connect(blur3, blur4, "blurred_image", "image");

    Microservice::Connect(blur3, blur5);
    Microservice::Connect(blur3, blur5, "blurred_image", "image");

    Run();
    LOG_SUCCESS("Workflow test completed");
  }
};
