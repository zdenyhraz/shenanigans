#pragma once
#include "Optimization_.h"

struct PatternSearch : public OptimizationAlgorithm_
{
  double minStep = 1e-5;
  int multistartMaxCnt = 1;
  int multistartCnt = 0;
  int maxExploitCnt = 0;
  double stepReducer = 0.5;

  PatternSearch(int N_) : OptimizationAlgorithm_(N_){};

  OptimizationResult Optimize(
      ObjectiveFunction obj, ValidationFunction valid = [](const std::vector<double>&) { return 0; }) override;
};
