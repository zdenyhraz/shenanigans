#include "StuffWindow.hpp"
#include "IPCWindow.hpp"
#include "Optimization/Evolution.hpp"
#include "Optimization/TestFunctions.hpp"
#include "UtilsCV/Vectmat.hpp"
#include "Random/ObjectDetection.hpp"
#include "Random/UnevenIllumination.hpp"

void StuffWindow::Initialize()
{
}

void StuffWindow::Render()
{
  if (ImGui::BeginTabItem("Stuff"))
  {
    ImGui::Separator();
    if (ImGui::Button("ImGui demo"))
      showImGuiDemoWindow = !showImGuiDemoWindow;
    if (ImGui::Button("ImPlot demo"))
      showImPlotDemoWindow = !showImPlotDemoWindow;

    ImGui::Separator();
    if (ImGui::Button("Evolution optimization"))
      LaunchAsync([]() { EvolutionOptimization(false); });
    if (ImGui::Button("Evolution meta optimization"))
      LaunchAsync([]() { EvolutionOptimization(true); });
    if (ImGui::Button("False correlations removal"))
      LaunchAsync([]() { FalseCorrelationsRemoval(); });

    ImGui::Separator();
    if (ImGui::Button("Plot test"))
      LaunchAsync([]() { PlotTest(); });

    ImGui::Separator();
    if (ImGui::Button("Uneven Illumination CLAHE"))
      LaunchAsync([]() { UnevenIlluminationCLAHE(); });
    if (ImGui::Button("Uneven Illumination Homomorphic"))
      LaunchAsync([]() { UnevenIlluminationHomomorphic(); });

    ImGui::Separator();
    if (ImGui::Button("Object Detection"))
      LaunchAsync([]() { ObjectDetection(); });

    ImGui::EndTabItem();
  }

  if (showImGuiDemoWindow)
    ImGui::ShowDemoWindow();
  if (showImPlotDemoWindow)
    ImPlot::ShowDemoWindow();
}

void StuffWindow::EvolutionOptimization(bool meta)
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
  Evo.mLB = Zerovect(N, -5.0);
  Evo.mUB = Zerovect(N, +5.0);
  Evo.mMaxFunEvals = maxFunEvals;
  Evo.mOptimalFitness = optimalFitness;
  Evo.SetName("debug");
  Evo.SetParameterNames({"x", "y"});
  Evo.SetConsoleOutput(true);
  Evo.SetPlotOutput(true);
  Evo.SetPlotObjectiveFunctionLandscape(false);
  Evo.SetPlotObjectiveFunctionLandscapeIterations(meta ? 51 : 301);
  Evo.SetSaveProgress(true);
  if (not meta)
    Evo.SetAllowInconsistent(true);

  if (meta)
    Evo.MetaOptimize(OptimizationTestFunctions::Rosenbrock, Evolution::ObjectiveFunctionValue, runs, maxFunEvals, optimalFitness);
  else
    Evo.Optimize(OptimizationTestFunctions::Rosenbrock, OptimizationTestFunctions::RosenbrockNoisy<noiseStddev>);
}

void StuffWindow::FalseCorrelationsRemoval()
{
  LOG_FUNCTION;
  auto& ipc = IPCWindow::GetIPC();
  auto image1 = LoadUnitFloatImage<IPC::Float>("../data/articles/swind/source/1/cropped/crop1.PNG");
  auto image2 = LoadUnitFloatImage<IPC::Float>("../data/articles/swind/source/1/cropped/crop9.PNG");

  if (false) // crop
  {
    const auto centerx = 0.1;
    const auto centery = 0.7;
    const auto size = 2 * centerx * image1.cols;
    image1 = RoiCrop(image1, centerx * image1.cols, centery * image1.rows, size, size);
    image2 = RoiCrop(image2, centerx * image2.cols, centery * image2.rows, size, size);
  }
  ipc.SetSize(image1.size());
  ipc.Calculate<IPC::Mode::Debug>(image1, image2);
}

void StuffWindow::PlotTest()
{
  const auto gaussian2D = Gaussian<f32>(101, Random::Rand<f32>(0, 31));
  const auto gaussian1D = GetMidRow<f64>(gaussian2D);
  const auto x = Iota<f64>(0, gaussian1D.size());

  PyPlot::Plot("1D", {.x = x, .y = gaussian1D});
  PyPlot::Plot("2D", {.z = gaussian2D});

  ImGuiPlot::Get().Plot("1D", PlotData1D{.x = x, .ys = {gaussian1D}});
  ImGuiPlot::Get().Plot("2D", PlotData2D{.z = gaussian2D});
}

void StuffWindow::ObjectDetection()
{
  LOG_FUNCTION;
  auto image = LoadUnitFloatImage<f32>(GetProjectDirectoryPath() / "data/debug/ObjectDetection/input/2.jpg");
  DetectObjectsEdge(image);
}

void StuffWindow::UnevenIlluminationCLAHE()
{
  LOG_FUNCTION;
  auto image = cv::imread("../data/debug/UnevenIllumination/input.jpg");
  const auto tileGridSize = 8;
  const auto clipLimit = 1;
  CorrectUnevenIlluminationCLAHE(image, tileGridSize, clipLimit);
}

void StuffWindow::UnevenIlluminationHomomorphic()
{
  LOG_FUNCTION;
  auto image = cv::imread("../data/debug/UnevenIllumination/input.jpg");
  for (auto cutoff = 0.001; cutoff <= 0.02; cutoff += 0.001)
    CorrectUnevenIlluminationHomomorphic(image, cutoff);
}
