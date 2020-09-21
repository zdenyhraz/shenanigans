#pragma once
#include "Optimization.h"

class Evolution : public OptimizationAlgorithm
{
public:
  Evolution(int N);
  std::vector<double> optimize(const std::function<double(const std::vector<double> &)> &f) override;

  enum MutationStrategy
  {
    RAND1,
    BEST1,
    RAND2,
    BEST2
  };

  enum CrossoverStrategy
  {
    BIN,
    EXP
  };

  enum StoppingCriterion
  {
    ALLIMP,
    AVGIMP
  };

  int mNP = 4;
  double mF = 0.65;
  double mCR = 0.95;
  double mINPm = 8;
  MutationStrategy mMutStrat = RAND1;
  CrossoverStrategy mCrossStrat = BIN;
  StoppingCriterion mStopCrit = AVGIMP;
  int mDistincEntityMaxTrials = 10;
  int mHistorySize = 10;
  double mHistoryImprovTresholdPercent = 1;

private:
};
