#include "MicroserviceEditorWindow.hpp"
#include "Microservice/LoadImageMicroservice.hpp"
#include "Microservice/BlurImageMicroservice.hpp"
#include "Microservice/PlotImageMicroservice.hpp"

void MicroserviceEditorWindow::Initialize()
{
  TestInitialize();
  editor.OnStart();
}

void MicroserviceEditorWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("MicroserviceEditor"))
  {
    ImGui::Separator();
    if (ImGui::Button("Run"))
      LaunchAsync([&]() { Run(); });

    editor.OnFrame();

    ImGui::EndTabItem();
  }
}

void MicroserviceEditorWindow::Run()
{
  editor.workflow.Run();
  LOG_SUCCESS("Workflow run completed");
}

void MicroserviceEditorWindow::TestInitialize()
{
  editor.workflow.AddMicroservice<LoadImageMicroservice>();
  editor.workflow.AddMicroservice<BlurImageMicroservice>();
  editor.workflow.AddMicroservice<BlurImageMicroservice>();
  editor.workflow.AddMicroservice<BlurImageMicroservice>();
  editor.workflow.AddMicroservice<PlotImageMicroservice>();
  editor.workflow.AddMicroservice<PlotImageMicroservice>();
  editor.workflow.AddMicroservice<PlotImageMicroservice>();

  const auto& microservices = editor.workflow.GetMicroservices();

  auto& start = *microservices[0];
  auto& load = *microservices[1];
  auto& blur3 = *microservices[2];
  auto& blur4 = *microservices[3];
  auto& blur5 = *microservices[4];
  auto& plot1 = *microservices[5];
  auto& plot2 = *microservices[6];
  auto& plot3 = *microservices[7];

  editor.workflow.Connect(start, blur3);
  editor.workflow.Connect(load, blur3, "image", "image");

  editor.workflow.Connect(blur3, blur4);
  editor.workflow.Connect(blur3, blur4, "blurred", "image");

  editor.workflow.Connect(blur3, blur5);
  editor.workflow.Connect(blur3, blur5, "blurred", "image");

  editor.workflow.Connect(blur4, plot1);
  editor.workflow.Connect(blur4, plot1, "blurred", "image");

  editor.workflow.Connect(blur5, plot2);
  editor.workflow.Connect(blur5, plot2, "blurred", "image");

  editor.workflow.Connect(load, plot3);
  editor.workflow.Connect(load, plot3, "image", "image");
}
