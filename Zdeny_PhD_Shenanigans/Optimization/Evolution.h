#pragma once
#include "Optimization.h"

class Evolution : public OptimizationAlgorithm
{
public:
  Evolution(int N, const std::string& optname = "noname");
  OptimizationResult Optimize(ObjectiveFunction obj, ValidationFunction valid = nullptr) override;
  void SetFileOutputDir(const std::string& dir);
  void SetPlotOutput(bool PlotOutput) { mPlotOutput = PlotOutput; }
  void SetConsoleOutput(bool ConsoleOutput) { mConsoleOutput = ConsoleOutput; }
  void SetParameterNames(const std::vector<std::string>& ParameterNames) { mParameterNames = ParameterNames; };
  void SetOptimizationName(const std::string& optname) { mOptimizationName = optname; }
  void MetaOptimize(ObjectiveFunction obj, int runsPerObj = 3, int maxFunEvals = 10000, double optimalFitness = 1e-6);

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

  int mNP = 4;
  double mF = 0.65;
  double mCR = 0.90;
  MutationStrategy mMutStrat = RAND1;
  CrossoverStrategy mCrossStrat = BIN;

private:
  struct Entity
  {
    Entity() = default;
    Entity(int N);

    std::vector<double> params;
    double fitness;
  };

  struct Offspring
  {
    Offspring() = default;
    Offspring(int N, int nParents);
    void UpdateDistinctParents(int eid, int NP);
    void UpdateCrossoverParameters(CrossoverStrategy crossoverStrategy, double CR);

    std::vector<double> params;
    double fitness;
    std::vector<int> parentIndices;
    std::vector<bool> crossoverParameters;
  };

  struct Population
  {
    Population(int NP, int N, ObjectiveFunction obj, const std::vector<double>& LB, const std::vector<double>& UB, int nParents, bool consoleOutput);
    void UpdateDistinctParents(int eid);
    void UpdateCrossoverParameters(int eid, CrossoverStrategy crossoverStrategy, double CR);
    void UpdateOffspring(int eid, MutationStrategy mutationStrategy, ObjectiveFunction obj, double F, const std::vector<double>& LB, const std::vector<double>& UB);
    void PerformSelection();
    void UpdatePopulationFunctionEvaluations();
    void UpdateOffspringFunctionEvaluations();
    void UpdateBestEntity();
    void UpdateTerminationCriterions(double relativeDifferenceThreshold);

    std::vector<Entity> entities;
    std::vector<Offspring> offspring;
    Entity bestEntity;
    int functionEvaluations;
    double averageFitness;
    double previousFitness;
    double absoluteDifference;
    double relativeDifference;
    int relativeDifferenceGenerationsOverThreshold;

  private:
    void InitializePopulation(int NP, int N, ObjectiveFunction obj, const std::vector<double>& LB, const std::vector<double>& UB);
    void InitializeBestEntity();
    void InitializeOffspring(int nParents);

    bool mConsoleOutput = true;
  };

  void CheckObjectiveFunctionNormality(ObjectiveFunction obj);
  void CheckValidationFunctionNormality(ValidationFunction valid);
  void CheckBounds();
  int GetNumberOfParents();
  void InitializeOutputs(ValidationFunction valid);
  void UninitializeOutputs(const Population& population, TerminationReason reason);
  void UpdateOutputs(int generation, const Population& population, ValidationFunction valid);
  TerminationReason CheckTerminationCriterions(const Population& population, int generation);
  std::string GetOutputFileString(int generation, const std::vector<double>& bestEntity, double bestFitness);

  bool mFileOutput = false;
  bool mPlotOutput = true;
  bool mConsoleOutput = true;
  double mAbsoluteDifferenceThreshold = 1e-10;
  double mRelativeDifferenceThreshold = 0.9;
  int mRelativeDifferenceGenerationsOverThresholdThreshold = 10;
  std::string mOptimizationName = "noname";
  std::string mOutputFileDir;
  std::ofstream mOutputFile;
  std::vector<std::string> mParameterNames;
};
