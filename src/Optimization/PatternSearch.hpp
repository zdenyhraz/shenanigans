#pragma once
#include "Optimization.hpp"

struct PatternSearch : public OptimizationAlgorithm
{
  double minStep = 1e-5;
  int multistartMaxCnt = 1;
  int multistartCnt = 0;
  int maxExploitCnt = 0;
  double stepReducer = 0.5;

  explicit PatternSearch(int N_) : OptimizationAlgorithm(N_) {}

  OptimizationResult Optimize(const ObjectiveFunction& obj, const std::optional<ObjectiveFunction>& valid = std::nullopt) override;
};
