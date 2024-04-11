#include "RandomWindow.hpp"
#include "Optimization/Evolution.hpp"
#include "Optimization/TestFunctions.hpp"
#include "Random/UnevenIllumination.hpp"
#include "Microservice/Workflow.hpp"
#include "Utils/FPSCounter.hpp"

void RandomWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("Random"))
  {
    ImGui::Separator();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Optimizers"))
    {
      if (ImGui::Button("Evolution opt"))
        LaunchAsync([&]() { EvolutionOptimization(false); });
      ImGui::SameLine();
      if (ImGui::Button("Evolution metaopt"))
        LaunchAsync([&]() { EvolutionOptimization(true); });
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Uneven illumination compensation"))
    {
      if (ImGui::Button("CLAHE"))
        LaunchAsync([&]() { UnevenIlluminationCLAHE(); });
      ImGui::SameLine();
      if (ImGui::Button("Homomorphic"))
        LaunchAsync([&]() { UnevenIlluminationHomomorphic(); });
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Microservice architecture"))
    {
      if (ImGui::Button("Test manual"))
        LaunchAsync([&]() { MicroserviceTestManual(); });
      ImGui::SameLine();
      if (ImGui::Button("Test nodes"))
        LaunchAsync([&]() { MicroserviceTestNodes(); });
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Networking via ZMQ"))
    {
      if (ImGui::Button("Video IPC"))
        LaunchAsync([&]() { NetworkingTestIPC(); });
      ImGui::SameLine();
      if (ImGui::Button("Video TCP"))
        LaunchAsync([&]() { NetworkingTestTCP(); });
    }
    ImGui::EndTabItem();
  }
}

void RandomWindow::EvolutionOptimization(bool meta) const
{
  LOG_FUNCTION;
  static constexpr i32 N = 2;
  static constexpr i32 runs = 20;
  static constexpr i32 maxFunEvals = 1000;
  static constexpr f64 optimalFitness = -std::numeric_limits<f64>::max();
  static constexpr f64 noiseStddev = 0.3;
  Evolution Evo(N);
  Evo.mNP = 5 * N;
  Evo.mMutStrat = Evolution::RAND1;
  Evo.SetLowerBounds(Zerovect(N, -5.0));
  Evo.SetUpperBounds(Zerovect(N, +5.0));
  Evo.SetMaxFunEvals(maxFunEvals);
  Evo.SetOptimalFitness(optimalFitness);
  Evo.SetName("debug");
  Evo.SetParameterNames({"x", "y"});
  Evo.SetConsoleOutput(true);
  Evo.SetPlotOutput(true);
  Evo.SetSaveProgress(true);

  if (meta)
    Evo.MetaOptimize(OptimizationTestFunctions::Rosenbrock, Evolution::ObjectiveFunctionValue, runs, maxFunEvals, optimalFitness);
  else
    Evo.Optimize(OptimizationTestFunctions::Rosenbrock, OptimizationTestFunctions::RosenbrockNoisy<noiseStddev>);
}

void RandomWindow::UnevenIlluminationCLAHE() const
{
  LOG_FUNCTION;
  auto image = cv::imread("../data/UnevenIllumination/input.jpg");
  const auto tileGridSize = 8;
  const auto clipLimit = 1;
  CorrectUnevenIlluminationCLAHE(image, tileGridSize, clipLimit);
}

void RandomWindow::UnevenIlluminationHomomorphic() const
{
  LOG_FUNCTION;
  auto image = cv::imread("../data/UnevenIllumination/input.jpg");
  for (auto cutoff = 0.001; cutoff <= 0.02; cutoff += 0.001)
    CorrectUnevenIlluminationHomomorphic(image, cutoff);
}

void RandomWindow::MicroserviceTestManual() const
{
  LOG_FUNCTION;
  MicroserviceRegistry::Initialize();
  Workflow::TestManual();
}

void RandomWindow::MicroserviceTestNodes() const
{
  LOG_FUNCTION;
  MicroserviceRegistry::Initialize();
  std::vector<Node> nodes;
  Workflow workflow("nodes");
  workflow.Build(nodes);
  workflow.Run();
}

void RandomWindow::NetworkingTestIPC() const
{
  LOG_FUNCTION;
  zmq::context_t context(1);
  zmq::socket_t subscriber(context, ZMQ_SUB);
  subscriber.connect("ipc:///video_feed");
  subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

  cv::Mat frame;
  cv::namedWindow("Received Video", cv::WINDOW_NORMAL);

  while (true)
  {
    zmq::message_t message;
    subscriber.recv(&message);
    LOG_DEBUG("Recieved message with size: {} ({})", message.size(), message.data());

    // Convert zmq::message_t to cv::Mat without copying data
    cv::Mat frame(1, message.size(), CV_8UC1, message.data());
    frame = frame.reshape(3);

    LOG_DEBUG("Mat size: {}x{}", frame.cols, frame.rows);
    cv::imshow("Received Video", frame);
    cv::waitKey(1);
  }
}

void RandomWindow::NetworkingTestTCP() const
{
  LOG_FUNCTION;
  zmq::context_t context(1);
  zmq::socket_t subscriber(context, ZMQ_SUB);
  subscriber.connect("tcp://localhost:5555");
  subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
  FPSCounter<20> fpscounter;

  while (true)
  {
    const auto start = std::chrono::high_resolution_clock::now();
    zmq::message_t message;
    subscriber.recv(&message);
    // LOG_DEBUG("Recieved message with size: {}", message.size());

    // frame = cv::imdecode(cv::Mat(1, message.size(), CV_8UC1, message.data()), cv::IMREAD_UNCHANGED);
    // frame.reshape(3, 480);
    cv::Mat frame = cv::Mat(cv::Size(1920, 1080), CV_8UC3, message.data(), cv::Mat::AUTO_STEP);
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::duration<float>>(end - start);
    fpscounter.RegisterDuration(duration.count());

    cv::putText(frame, fmt::format("{:.0f}", fpscounter.GetFPS()), cv::Point(frame.cols * 0.97, frame.cols * 0.02), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 3);
    cv::imshow("Received Video", frame);
    cv::waitKey(1);
  }

  cv::destroyAllWindows();
}
