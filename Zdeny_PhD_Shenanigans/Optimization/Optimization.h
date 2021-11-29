#pragma once
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Log/logger.h"

class OptimizationAlgorithm
{
public:
  enum TerminationReason
  {
    NotTerminated = 0,
    OptimalFitnessReached,
    MaximumGenerationsReached,
    MaximumFunctionEvaluationsReached,
    NoImprovementReachedRel,
    NoImprovementReachedAbs,
    UnexpectedErrorOccured,
  };

  using ObjectiveFunction = const std::function<double(const std::vector<double>&)>&;
  using ValidationFunction = ObjectiveFunction;
  using Optimum = std::vector<double>;

  struct OptimizationResult
  {
    Optimum optimum;
    TerminationReason terminationReason = NotTerminated;
    int functionEvaluations = -1;
  };

  OptimizationAlgorithm(int N);
  virtual ~OptimizationAlgorithm() = default;

  virtual OptimizationResult Optimize(
      ObjectiveFunction obj, ValidationFunction valid = [](const std::vector<double>&) { return 0; }) = 0;

  static void PlotObjectiveFunctionLandscape(ObjectiveFunction f, const std::vector<double> baseParams, int iters, int xParamIndex, int yParamIndex, double xmin, double xmax, double ymin, double ymax,
      const std::string& xName, const std::string& yName, bool applyLog);

  int N = 1;                               // the problem dimension
  std::vector<double> mLB;                 // lower search space bounds
  std::vector<double> mUB;                 // upper search space bounds
  double optimalFitness = -Constants::Inf; // satisfactory function value
  int maxFunEvals = Constants::IntInf;     // maximum # of function evaluations
  int maxGen = 1000;                       // maximum # of algorithm iterations

protected:
  std::string GetTerminationReasonString(const TerminationReason& reason);
};
