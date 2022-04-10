#include "StuffWindow.hpp"
#include "Optimization/Evolution.hpp"
#include "Optimization/TestFunctions.hpp"

void StuffWindow::Initialize()
{
}

void StuffWindow::Render()
{
  if (ImGui::BeginTabItem("Stuff"))
  {
    if (ImGui::Button("Evolution optimization"))
      LaunchAsync([]() { EvolutionOptimization(false); });

    if (ImGui::Button("Evolution meta optimization"))
      LaunchAsync([]() { EvolutionOptimization(true); });

    ImGui::EndTabItem();
  }
}

void StuffWindow::EvolutionOptimization(bool meta)
{
  static constexpr i32 N = 2;
  static constexpr i32 runs = 20;
  static constexpr i32 maxFunEvals = 1000;
  static constexpr f64 optimalFitness = -std::numeric_limits<f64>::max();
  static constexpr f64 noiseStddev = 0.5;
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
  Evo.SetPlotObjectiveFunctionLandscape(true);
  Evo.SetPlotObjectiveFunctionLandscapeIterations(meta ? 51 : 301);
  Evo.SetSaveProgress(true);
  if (not meta)
    Evo.SetAllowInconsistent(true);

  if (meta)
    Evo.MetaOptimize(OptimizationTestFunctions::Rosenbrock, Evolution::ObjectiveFunctionValue, runs, maxFunEvals, optimalFitness);
  else
    Evo.Optimize(OptimizationTestFunctions::Rosenbrock, OptimizationTestFunctions::RosenbrockNoisy<noiseStddev>);
}
