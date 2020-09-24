#include "stdafx.h"
#include "Optimization.h"

OptimizationAlgorithm::OptimizationAlgorithm(int N) : N(N)
{
  mLB = zerovect(N, -1.);
  mUB = zerovect(N, 1.);
}

OptimizationAlgorithm::~OptimizationAlgorithm() {}

std::string OptimizationAlgorithm::GetTerminationReasonString(const TerminationReason &reason)
{
  switch (reason)
  {
  case OptimalFitnessReached:
    return "Optimal fitness reached";
  case MaximumGenerationsReached:
    return "Maximum generations reached";
  case MaximumFunctionEvaluationsReached:
    return "Maximum function evaluations reached";
  case NoImprovementReachedRel:
    return "No relative improvement reached";
  case NoImprovementReachedAbs:
    return "No absolute improvement reached";
  case NotTerminated:
    return "Not yet terminated";
  }
  return "";
};
