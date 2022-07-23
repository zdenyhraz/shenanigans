#include "StuffWindow.hpp"
#include "IPCWindow.hpp"
#include "Optimization/Evolution.hpp"
#include "Optimization/TestFunctions.hpp"
#include "Random/IntervalMap.hpp"

void StuffWindow::Initialize()
{
}

void StuffWindow::Render()
{
  if (ImGui::BeginTabItem("Stuff"))
  {
    ImGui::Separator();

    if (ImGui::Button("Evolution optimization"))
      LaunchAsync([]() { EvolutionOptimization(false); });

    if (ImGui::Button("Evolution meta optimization"))
      LaunchAsync([]() { EvolutionOptimization(true); });

    if (ImGui::Button("False correlations removal"))
      LaunchAsync([]() { FalseCorrelationsRemoval(); });

    if (ImGui::Button("Interval map test"))
      LaunchAsync([]() { IntervalMap<int, std::string>::Test(); });

    ImGui::EndTabItem();
  }
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
