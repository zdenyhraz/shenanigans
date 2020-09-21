#pragma once
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Log/logger.h"

class OptimizationAlgorithm // the main parent optimizer class
{
public:
  int N = 1;                                                                                          // the problem dimension
  std::vector<double> lowerBounds = zerovect(N, -1.);                                                 // lower search space bounds
  std::vector<double> upperBounds = zerovect(N, +1.);                                                 // upper search space bounds
  double optimalFitness = 0;                                                                          // satisfactory function value
  int funEvals = 0;                                                                                   // current # of function evaluations
  int maxFunEvals = 1e10;                                                                             // maximum # of function evaluations
  int maxGen = 1000;                                                                                  // maximum # of algorithm iterations
  bool success = false;                                                                               // success in reaching satisfactory function value
  bool logPoints = false;                                                                             // switch for logging of explored points
  std::vector<vector<vector<double>>> visitedPoints;                                                  // the 3D vector of visited points [run][iter][dim]
  std::string terminationReason = "optimization not run yet";                                         // the reason for algorithm termination
  OptimizationAlgorithm(int N) : N(N), lowerBounds(zerovect(N, -1.)), upperBounds(zerovect(N, 1.)){}; // construct some default bounds

  virtual std::vector<double> optimize(const std::function<double(const std::vector<double> &)> &f) = 0;

protected:
};
