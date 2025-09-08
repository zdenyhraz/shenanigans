#include "MicroserviceEditorWindow.hpp"
#include "Microservice/Microservices/LoadImage.hpp"
#include "Microservice/Microservices/SaveImage.hpp"
#include "Microservice/Microservices/PlotImage.hpp"
#include "Microservice/Microservices/DrawObjects.hpp"
#include "Microservice/Microservices/OnnxDetection.hpp"
#include "Utils/Async.hpp"

void MicroserviceEditorWindow::Initialize()
{
  editor.OnStart();
  CreateWorkflow();
  LOG_DEBUG("OpenCV version:\n{}", cv::getBuildInformation());
}

void MicroserviceEditorWindow::Render()
{
  PROFILE_FUNCTION;

  if (ImGui::BeginTabItem("MicroserviceEditor"))
  {
    ImGui::Separator();
    if (ImGui::Button("Run"))
      Run();
    ImGui::SameLine();
    if (ImGui::Button("Run async"))
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
  LOG_SCOPE("Workflow Load+Run");
  editor.workflow.Load();
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

void MicroserviceEditorWindow::CreateWorkflow()
{
  auto& load = editor.workflow.AddMicroservice<LoadImage>();
  auto& onnx = editor.workflow.AddMicroservice<OnnxDetection>();
  auto& draw = editor.workflow.AddMicroservice<DrawObjects>();
  auto& save = editor.workflow.AddMicroservice<SaveImage>();

  load.SetParameter("image path", std::string("data/umbellula/benchmark/umbellula.jpg"));
  onnx.SetParameter("model path", std::string("data/umbellula/umbellula.onnx"));
  save.SetParameter("image path", std::string("data/umbellula/benchmark/result_cpp.jpg"));

  editor.workflow.Connect(editor.workflow.GetStart(), onnx);
  editor.workflow.Connect(load, onnx, "image");
  editor.workflow.Connect(load, draw, "image");
  editor.workflow.Connect(onnx, save);
  editor.workflow.Connect(onnx, draw, "objects");
  editor.workflow.Connect(draw, save, "image");
}

void MicroserviceEditorWindow::ShowFlow()
{
  editor.ShowFlow();
}
