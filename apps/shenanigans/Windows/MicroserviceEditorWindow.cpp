#include "MicroserviceEditorWindow.hpp"
#include "Microservice/Microservices/LoadImageMicroservice.hpp"
#include "Microservice/Microservices/BlurImageMicroservice.hpp"
#include "Microservice/Microservices/PlotImageMicroservice.hpp"
#include "NDA/Microservice/ObjdetectSASMicroservice.hpp"
#include "Microservice/Microservices/PlotObjectsMicroservice.hpp"
#include "Utils/Async.hpp"

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
    ImGui::SameLine();
    if (ImGui::Button("Run repeat"))
      LaunchAsync([&]() { RunRepeat(); });
    ImGui::SameLine();
    if (ImGui::Button("Show flow"))
      editor.ShowFlow();

    editor.OnFrame();

    ImGui::EndTabItem();
  }
}

void MicroserviceEditorWindow::Run()
{
  editor.workflow.Run();
  LOG_SUCCESS("Workflow run completed");
}

void MicroserviceEditorWindow::RunRepeat()
{
  for (int i = 0; i < 5; ++i)
  {
    Plot::Clear();
    editor.workflow.Run();
  }
  LOG_SUCCESS("Workflow run completed");
}

void MicroserviceEditorWindow::TestInitialize()
{
  editor.workflow.AddMicroservice<LoadImageMicroservice>();
  editor.workflow.AddMicroservice<BlurImageMicroservice>();
  editor.workflow.AddMicroservice<PlotImageMicroservice>();
  editor.workflow.AddMicroservice<ObjdetectSASMicroservice>();
  editor.workflow.AddMicroservice<PlotObjectsMicroservice>();

  const auto& microservices = editor.workflow.GetMicroservices();
  auto& start = *microservices[0];
  auto& load = *microservices[1];
  auto& blur = *microservices[2];
  auto& plotBlur2 = *microservices[3];
  auto& sas = *microservices[4];
  auto& plotSAS = *microservices[5];

  load.SetParameter("file name", std::string("rox.tif"));
  load.SetParameter("float", true);
  load.SetParameter("grayscale", true);
  blur.SetParameter("relative blur x", 0.0f);
  blur.SetParameter("relative blur y", 0.0f);

  editor.workflow.Connect(start, blur);
  editor.workflow.Connect(load, blur, "image", "image");
  editor.workflow.Connect(blur, plotBlur2);
  editor.workflow.Connect(blur, plotBlur2, "blurred", "image");
  editor.workflow.Connect(blur, sas, "blurred", "image");
  editor.workflow.Connect(blur, sas);
  editor.workflow.Connect(sas, plotSAS);
  editor.workflow.Connect(sas, plotSAS, "objects", "objects");
  editor.workflow.Connect(load, plotSAS, "image", "image");
}

void MicroserviceEditorWindow::ShowFlow()
{
  editor.ShowFlow();
}
