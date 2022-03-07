#pragma once
#include "Optimization.hpp"

struct PatternSearch : public OptimizationAlgorithm
{
  f64 minStep = 1e-5;
  i32 multistartMaxCnt = 1;
  i32 multistartCnt = 0;
  i32 maxExploitCnt = 0;
  f64 stepReducer = 0.5;

  explicit PatternSearch(i32 N_) : OptimizationAlgorithm(N_) {}

  OptimizationResult Optimize(
      ObjectiveFunction obj, ValidationFunction valid = [](const std::vector<f64>&) { return 0; }) override;
};
