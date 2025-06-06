#include "RandomWindow.hpp"
#include "Optimization/Evolution.hpp"
#include "Optimization/TestFunctions.hpp"
#include "Random/UnevenIllumination.hpp"
#include "Utils/FrameAverager.hpp"
#include "Utils/Async.hpp"

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
    ImGui::EndTabItem();
  }
}

void RandomWindow::EvolutionOptimization(bool meta)
{
  LOG_FUNCTION;
  static constexpr int N = 2;
  static constexpr int runs = 20;
  static constexpr int maxFunEvals = 1000;
  static constexpr double optimalFitness = -std::numeric_limits<double>::max();
  static constexpr double noiseStddev = 0.3;
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
    Evo.Optimize(OptimizationTestFunctions::Rosenbrock, [](const auto& vec) { return OptimizationTestFunctions::RosenbrockNoisy(vec, noiseStddev); });
}

void RandomWindow::UnevenIlluminationCLAHE()
{
  LOG_FUNCTION;
  auto image = cv::imread("../data/UnevenIllumination/input.jpg");
  const auto tileGridSize = 8;
  const auto clipLimit = 1;
  CorrectUnevenIlluminationCLAHE(image, tileGridSize, clipLimit);
}

void RandomWindow::UnevenIlluminationHomomorphic()
{
  LOG_FUNCTION;
  auto image = cv::imread("../data/UnevenIllumination/input.jpg");
  for (auto cutoff = 0.001; cutoff <= 0.02; cutoff += 0.001)
    CorrectUnevenIlluminationHomomorphic(image, cutoff);
}
