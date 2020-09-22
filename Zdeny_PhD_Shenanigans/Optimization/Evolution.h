#pragma once
#include "Optimization.h"

class Evolution : public OptimizationAlgorithm
{
public:
  Evolution(int N);
  std::tuple<std::vector<double>, TerminationReason> optimize(const std::function<double(const std::vector<double> &)> &f) override;
  void SetFileOutput(const std::string &path);
  void SetPlotOutput(bool PlotOutput) { mPlotOutput = PlotOutput; };
  void SetParameterNames(const std::vector<std::string> &ParameterNames) { mParameterNames = ParameterNames; };

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
  std::vector<std::string> mParameterNames;
  bool mFileOutput = false;
  bool mPlotOutput = true;
  std::string mOutputFilePath;
  std::ofstream mOutputFile;

private:
  bool InitializeOutputs();
  int GetNumberOfParents();
  std::vector<std::vector<double>> InitializePopulation();
  std::vector<double> InitializeFitness(const std::vector<std::vector<double>> &population, const std::function<double(const std::vector<double> &)> &f);
  std::pair<std::vector<double>, double> InitializeBestEntity(const std::vector<std::vector<double>> &population, const std::vector<double> &populationFitness);
  bool CheckObjectiveFunctionNormality(const std::function<double(const std::vector<double> &)> &f);
  void CalculateDistinctParents(int entityIndex, std::vector<int> &parentIndices);
  void CalculateCrossoverParameters(std::vector<bool> &crossoverParameters);
  void CalculateOffspring(const std::vector<std::vector<double>> &population, const std::vector<double> &bestEntity, const std::vector<int> &parentIndices, const std::vector<bool> &crossoverParameters, int indexEntity, std::vector<double> &offspring);
  void CalculateOffspringFitness(const std::vector<double> &offspring, const std::function<double(const std::vector<double> &)> &f, double &offspringFitness);
  void CalculateSelection(const std::vector<double> &offspring, double offspringFitness, std::vector<double> &population, double &populationFitness);
  void UpdateFunctionEvaluations(int &functionEvaluations);
  void CalculateBestEntity(const std::vector<std::vector<double>> &population, const std::vector<double> &populationFitness, std::vector<double> &bestEntity, double &bestFitness);
  void UpdateOutputs(int generation, const std::vector<double> &bestEntity, double bestFitness, double averageImprovement, double &fitnessPrevious, double &fitnessCurrent);
  void UpdateHistories(const std::vector<double> &populationFitness, std::vector<std::queue<double>> &populationHistory, double &averageImprovement, bool &historyConstant);
  std::pair<bool, TerminationReason> CheckTerminationCriterions(double bestFitness, int generation, int functionEvaluations, bool historyConstant);
  std::string GetOutputFileString(int generation, const std::vector<double> &bestEntity, double bestFitness);
};
