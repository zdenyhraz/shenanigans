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

  OptimizationAlgorithm(int N);
  virtual ~OptimizationAlgorithm();
  virtual std::tuple<std::vector<double>, TerminationReason> optimize(const std::function<double(const std::vector<double> &)> &f) = 0;

  int N = 1;                                          // the problem dimension
  std::vector<double> lowerBounds = zerovect(N, -1.); // lower search space bounds
  std::vector<double> upperBounds = zerovect(N, +1.); // upper search space bounds
  double optimalFitness = 0;                          // satisfactory function value
  int maxFunEvals = 1e10;                             // maximum # of function evaluations
  int maxGen = 1000;                                  // maximum # of algorithm iterations

protected:
};
