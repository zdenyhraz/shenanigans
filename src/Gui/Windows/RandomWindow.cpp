#include "RandomWindow.hpp"
#include "Optimization/Evolution.hpp"
#include "Optimization/TestFunctions.hpp"
#include "Random/UnevenIllumination.hpp"

void RandomWindow::Render()
{
  PROFILE_FUNCTION;
  if (ImGui::BeginTabItem("Random"))
  {
    ImGui::Separator();

    ImGui::BulletText("Optimizers");
    if (ImGui::Button("Evolution opt"))
      LaunchAsync([&]() { EvolutionOptimization(false); });
    ImGui::SameLine();
    if (ImGui::Button("Evolution metaopt"))
      LaunchAsync([&]() { EvolutionOptimization(true); });

    ImGui::BulletText("Uneven illumination compensation");
    if (ImGui::Button("CLAHE"))
      LaunchAsync([&]() { UnevenIlluminationCLAHE(); });
    ImGui::SameLine();
    if (ImGui::Button("Homomorphic"))
      LaunchAsync([&]() { UnevenIlluminationHomomorphic(); });

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

void RandomWindow::UnevenIlluminationCLAHE() const
{
  LOG_FUNCTION;
  auto image = cv::imread("../data/debug/UnevenIllumination/input.jpg");
  const auto tileGridSize = 8;
  const auto clipLimit = 1;
  CorrectUnevenIlluminationCLAHE(image, tileGridSize, clipLimit);
}

void RandomWindow::UnevenIlluminationHomomorphic() const
{
  LOG_FUNCTION;
  auto image = cv::imread("../data/debug/UnevenIllumination/input.jpg");
  for (auto cutoff = 0.001; cutoff <= 0.02; cutoff += 0.001)
    CorrectUnevenIlluminationHomomorphic(image, cutoff);
}