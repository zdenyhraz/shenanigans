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

  int N = 1;                               // the problem dimension
  std::vector<double> lowerBounds;         // lower search space bounds
  std::vector<double> upperBounds;         // upper search space bounds
  double optimalFitness = -Constants::Inf; // satisfactory function value
  int maxFunEvals = Constants::IntInf;     // maximum # of function evaluations
  int maxGen = 1000;                       // maximum # of algorithm iterations

protected:
  std::string GetTerminationReasonString(const TerminationReason &reason);
};
