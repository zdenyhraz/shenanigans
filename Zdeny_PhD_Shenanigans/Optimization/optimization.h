#pragma once
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Log/logger.h"

class OptimizationAlgorithm // the main parent optimizer class
{
public:
  enum TerminationReason
  {
    OptimalFitnessReached,
    MaximumGenerationsReached,
    MaximumFunctionEvaluationsReached,
    NoImprovementReached,
    NotTerminated
  };

  using ObjectiveFunction = const std::function<double(const std::vector<double> &)> &;
  using Optimum = std::vector<double>;
  using OptimizationResult = std::tuple<Optimum, TerminationReason>;

  OptimizationAlgorithm(int N);
  virtual ~OptimizationAlgorithm();
  virtual OptimizationResult Optimize(ObjectiveFunction f) = 0;

  int N = 1;                               // the problem dimension
  std::vector<double> lowerBounds;         // lower search space bounds
  std::vector<double> upperBounds;         // upper search space bounds
  double optimalFitness = -Constants::Inf; // satisfactory function value
  int maxFunEvals = Constants::IntInf;     // maximum # of function evaluations
  int maxGen = 1000;                       // maximum # of algorithm iterations

protected:
  std::string GetTerminationReasonString(const TerminationReason &reason);
};
