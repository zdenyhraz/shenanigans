#pragma once
#include "Optimization.h"

class Evolution : public OptimizationAlgorithm
{
public:
  enum MutationStrategy : char
  {
    RAND1,
    BEST1,
    RAND2,
    BEST2
  };
  enum CrossoverStrategy : char
  {
    BIN,
    EXP
  };
  enum StoppingCriterion : char
  {
    ALLIMP,
    AVGIMP
  };
  int NP = 4;
  double F = 0.65;
  double CR = 0.95;
  double iNPm = 8;
  MutationStrategy mutStrat = RAND1;
  CrossoverStrategy crossStrat = BIN;
  StoppingCriterion stopCrit = AVGIMP;
  int distincEntityMaxTrials = 10;
  int historySize = 10;
  double historyImprovTresholdPercent = 1;
  Evolution(int N) : OptimizationAlgorithm(N), NP(iNPm * N){};

  std::vector<double> optimize(const std::function<double(const std::vector<double> &)> &f) override;

private:
};
