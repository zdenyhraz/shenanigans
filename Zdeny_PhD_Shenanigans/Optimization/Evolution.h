#pragma once
#include "Optimization.h"

class Evolution : public OptimizationAlgorithm
{
public:
  Evolution(int N);
  OptimizationResult Optimize(ObjectiveFunction f) override;
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
  MutationStrategy mMutStrat = RAND1;
  CrossoverStrategy mCrossStrat = BIN;
  StoppingCriterion mStopCrit = AVGIMP;
  double mHistoryImprovTresholdPercent = 1.0;

private:
  struct Entity
  {
    Entity();
    Entity(int N);

    std::vector<double> params;
    double fitness;
    std::queue<double> fitnessHistory;
  };

  struct Offspring
  {
    Offspring();
    Offspring(int N, int nParents);
    void UpdateDistinctParents(int eid, int NP);
    void UpdateCrossoverParameters(int N, CrossoverStrategy crossoverStrategy, double CR);

    std::vector<double> params;
    double fitness;
    std::vector<int> parentIndices;
    std::vector<bool> crossoverParameters;
  };

  struct Population
  {
    Population();
    bool Initialize(int NP, int N, ObjectiveFunction f, const std::vector<double> &LB, const std::vector<double> &UB, int nParents);
    void UpdateDistinctParents(int eid, int NP);
    void UpdateCrossoverParameters(int eid, int N, CrossoverStrategy crossoverStrategy, double CR);
    void UpdateOffspring(int eid, int N, MutationStrategy mutationStrategy, ObjectiveFunction f, double F, const std::vector<double> &LB, const std::vector<double> &UB);
    void PerformSelection(int NP);
    void UpdateFunctionEvaluations(int NP);
    void UpdateBestEntity(int NP);
    void UpdateFitnessHistories(int NP, int nHistories, StoppingCriterion stoppingCriterion, double improvThreshold);

    std::vector<Entity> population;
    std::vector<Offspring> offspring;
    Entity bestEntity;
    int functionEvaluations;
    double previousBestFitness;
    double currentBestFitness;
    double improvement;
    bool constantHistory;

  private:
    void InitializePopulation(int NP, int N, ObjectiveFunction f, const std::vector<double> &LB, const std::vector<double> &UB);
    void InitializeOffspring(int NP, int N, ObjectiveFunction f, int nParents);
    void InitializeBestEntity(int NP, int N);
  };

  bool CheckObjectiveFunctionNormality(const std::function<double(const std::vector<double> &)> &f);
  int GetNumberOfParents();
  void InitializeOutputs(const Population &population);
  void UpdateOutputs(int generation, const Population &population);
  std::pair<bool, TerminationReason> CheckTerminationCriterions(double bestFitness, int generation, int functionEvaluations, bool historyConstant);
  std::string GetOutputFileString(int generation, const std::vector<double> &bestEntity, double bestFitness);

  bool mFileOutput = false;
  bool mPlotOutput = true;
  std::string mOutputFilePath;
  std::ofstream mOutputFile;
  std::vector<std::string> mParameterNames;
  double mINPm = 5.4;
  int mHistorySize = 10;
};
