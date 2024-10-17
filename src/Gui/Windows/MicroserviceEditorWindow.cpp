#include "MicroserviceEditorWindow.hpp"
#include "Microservice/Microservices/LoadImageMicroservice.hpp"
#include "Microservice/Microservices/BlurImageMicroservice.hpp"
#include "Microservice/Microservices/PlotImageMicroservice.hpp"
#include "NDA/SAS/ObjdetectSASMicroservice.hpp"
#include "Microservice/Microservices/PlotObjectsMicroservice.hpp"

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
  editor.workflow.AddMicroservice<BlurImageMicroservice>();
  editor.workflow.AddMicroservice<BlurImageMicroservice>();
  editor.workflow.AddMicroservice<PlotImageMicroservice>();
  editor.workflow.AddMicroservice<PlotImageMicroservice>();
  editor.workflow.AddMicroservice<PlotImageMicroservice>();
  editor.workflow.AddMicroservice<ObjdetectSASMicroservice>();
  editor.workflow.AddMicroservice<LoadImageMicroservice>();
  editor.workflow.AddMicroservice<PlotObjectsMicroservice>();

  const auto& microservices = editor.workflow.GetMicroservices();

  auto& start = *microservices[0];
  auto& load1 = *microservices[1];
  auto& blur3 = *microservices[2];
  auto& blur4 = *microservices[3];
  auto& blur5 = *microservices[4];
  auto& plot1 = *microservices[5];
  auto& plot2 = *microservices[6];
  auto& plot3 = *microservices[7];
  auto& sas = *microservices[8];
  auto& loadSAS = *microservices[9];
  auto& plotSAS = *microservices[10];

  loadSAS.SetParameter("file name", std::string("data/cx/sas/kd/SMK Test 0/sasi-S-upper-20221102-143803-l38.tif"));
  loadSAS.SetParameter("float", true);
  loadSAS.SetParameter("grayscale", true);

  editor.workflow.Connect(start, blur3);
  editor.workflow.Connect(start, plotSAS);

  editor.workflow.Connect(load1, blur3, "image", "image");

  editor.workflow.Connect(blur3, blur4);
  editor.workflow.Connect(blur3, blur4, "blurred", "image");

  editor.workflow.Connect(blur3, blur5);
  editor.workflow.Connect(blur3, blur5, "blurred", "image");

  editor.workflow.Connect(blur4, plot1);
  editor.workflow.Connect(blur4, plot1, "blurred", "image");

  editor.workflow.Connect(blur5, plot2, "blurred", "image");

  editor.workflow.Connect(load1, plot3);
  editor.workflow.Connect(load1, plot3, "image", "image");

  editor.workflow.Connect(sas, plotSAS, "objects", "objects");

  editor.workflow.Connect(loadSAS, sas, "image", "image");
  editor.workflow.Connect(loadSAS, plotSAS, "image", "image");
}

void MicroserviceEditorWindow::ShowFlow()
{
  editor.ShowFlow();
}
