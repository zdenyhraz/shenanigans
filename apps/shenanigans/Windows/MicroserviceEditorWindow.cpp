#include "MicroserviceEditorWindow.hpp"
#include "Microservice/Microservices/LoadImage.hpp"
#include "Microservice/Microservices/SaveImage.hpp"
#include "Microservice/Microservices/PlotImage.hpp"
#include "Microservice/Microservices/DrawObjects.hpp"
#include "Microservice/Microservices/OnnxDetection.hpp"
#include "NDA/Microservice/ObjdetectSAS.hpp"
#include "Utils/Async.hpp"

void MicroserviceEditorWindow::Initialize()
{
  editor.OnStart();
  TestInitialize();
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
}

void MicroserviceEditorWindow::RunRepeat()
{
  for (int i = 0; i < 5; ++i)
  {
    Plot::Clear();
    editor.workflow.Run();
  }
}

void MicroserviceEditorWindow::TestInitialize()
{
  auto& load = editor.workflow.AddMicroservice<LoadImage>();
  auto& onnx = editor.workflow.AddMicroservice<OnnxDetection>();
  auto& draw = editor.workflow.AddMicroservice<DrawObjects>();
  auto& save = editor.workflow.AddMicroservice<SaveImage>();

  load.SetParameter("image path", std::string("data/umbellula/interesting/0.jpg"));
  onnx.SetParameter("model path", std::string("data/umbellula/umbellula.onnx"));
  save.SetParameter("image path", std::string("data/umbellula/results/0.jpg"));

  editor.workflow.Connect(editor.workflow.GetStart(), onnx);
  editor.workflow.Connect(load, onnx, "image");
  editor.workflow.Connect(load, draw, "image");
  editor.workflow.Connect(onnx, save);
  editor.workflow.Connect(onnx, draw, "objects");
  editor.workflow.Connect(draw, save, "image");
  editor.workflow.Load();
}

void MicroserviceEditorWindow::ShowFlow()
{
  editor.ShowFlow();
}
