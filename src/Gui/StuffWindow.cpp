#include "StuffWindow.hpp"
#include "IPCWindow.hpp"
#include "Optimization/Evolution.hpp"
#include "Optimization/TestFunctions.hpp"
#include "UtilsCV/Vectmat.hpp"
#include "Random/UnevenIllumination.hpp"

void StuffWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("Stuff"))
  {
    ImGui::Separator();
    ImGui::Text("Demos");
    if (ImGui::Button("ImGui demo"))
      showImGuiDemoWindow = !showImGuiDemoWindow;
    ImGui::SameLine();
    if (ImGui::Button("ImPlot demo"))
      showImPlotDemoWindow = !showImPlotDemoWindow;

    ImGui::Text("Optimizers");
    if (ImGui::Button("Evolution opt"))
      LaunchAsync([&]() { EvolutionOptimization(false); });
    ImGui::SameLine();
    if (ImGui::Button("Evolution metaopt"))
      LaunchAsync([&]() { EvolutionOptimization(true); });

    ImGui::Text("PyPlot & ImGuiPlot & CvPlot");
    if (ImGui::Button("Plot test"))
      LaunchAsync([&]() { PlotTest(); });

    ImGui::Text("Uneven illumination compensation");
    if (ImGui::Button("CLAHE"))
      LaunchAsync([&]() { UnevenIlluminationCLAHE(); });
    ImGui::SameLine();
    if (ImGui::Button("Homomorphic"))
      LaunchAsync([&]() { UnevenIlluminationHomomorphic(); });

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

void StuffWindow::PlotTest()
{
  const int n = 101;
  const auto gaussian2D = Random::Rand<f32>(0, 1) * Gaussian<f32>(n, Random::Rand<f32>(0, 0.5) * n);
  const auto gaussian1D = GetMidRow<f32>(gaussian2D);
  const auto x = Iota<f64>(0, gaussian1D.size());

  ImGuiPlot::Plot({.name = "ig1D", .x = x, .ys = {gaussian1D}});
  ImGuiPlot::Plot({.name = "ig2D", .z = gaussian2D});

  PyPlot::Plot({.name = "py1D", .x = x, .ys = {gaussian1D}});
  PyPlot::Plot({.name = "py2D", .z = gaussian2D});

  CvPlot::Plot({.name = "cv1D", .x = x, .ys = {gaussian1D}});
  CvPlot::Plot({.name = "cv2D", .z = gaussian2D});
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
