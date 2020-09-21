#include "stdafx.h"
#include "Optimization.h"

OptimizationAlgorithm::OptimizationAlgorithm(int N) : N(N)
{
  lowerBounds = zerovect(N, -1.);
  upperBounds = zerovect(N, 1.);
}

OptimizationAlgorithm::~OptimizationAlgorithm(){};
