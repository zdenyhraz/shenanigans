#pragma once
#include "Optimization_.h"

class Evolution : public OptimizationAlgorithm_
{
public:
  enum MutationStrategy
  {
    RAND1,
    RAND2,
    BEST1,
    BEST2,
    MutationStrategyCount
  };

  enum CrossoverStrategy
  {
    BIN,
    EXP,
    CrossoverStrategyCount
  };

  enum MetaObjectiveFunctionType
  {
    ObjectiveFunctionValue
  };

  Evolution(usize N, const std::string& optname = "default");
  OptimizationResult Optimize(ObjectiveFunction obj, ValidationFunction valid = nullptr) override;
  void MetaOptimize(ObjectiveFunction obj, MetaObjectiveFunctionType metaObjType = ObjectiveFunctionValue, usize runsPerObj = 3, usize maxFunEvals = 10000, f64 optimalFitness = -Constants::Inf);

  usize mNP = 4;
  f64 mF = 0.65;
  f64 mCR = 0.90;
  MutationStrategy mMutStrat = RAND1;
  CrossoverStrategy mCrossStrat = BIN;

private:
  struct Entity
  {
    Entity() = default;
    Entity(usize N);

    std::vector<f64> params;
    f64 fitness;
  };

  struct Offspring
  {
    Offspring() = default;
    Offspring(usize N, usize nParents);
    void UpdateDistinctParents(usize eid, usize NP);
    void UpdateCrossoverParameters(CrossoverStrategy crossoverStrategy, f64 CR);

    std::vector<f64> params;
    f64 fitness;
    std::vector<usize> parentIndices;
    std::vector<bool> crossoverParameters;
  };

  struct Population
  {
    Population(usize NP, usize N, ObjectiveFunction obj, const std::vector<f64>& LB, const std::vector<f64>& UB, usize nParents, bool consoleOutput, bool saveProgress);
    void UpdateDistinctParents(usize eid);
    void UpdateCrossoverParameters(usize eid, CrossoverStrategy crossoverStrategy, f64 CR);
    void UpdateOffspring(usize eid, MutationStrategy mutationStrategy, ObjectiveFunction obj, f64 F, const std::vector<f64>& LB, const std::vector<f64>& UB);
    void PerformSelection();
    void UpdateBestEntity();
    void UpdateTerminationCriterions(f64 relativeDifferenceThreshold);
    void UpdateProgress();

    std::vector<Entity> entities;
    std::vector<Offspring> offspring;
    Entity bestEntity;
    usize functionEvaluations;
    f64 averageFitness;
    f64 previousFitness;
    f64 absoluteDifference;
    f64 relativeDifference;
    usize relativeDifferenceGenerationsOverThreshold;
    bool mSaveProgress;
    std::vector<f64> bestFitnessProgress;
    std::vector<std::vector<f64>> bestParametersProgress;
    std::vector<std::vector<f64>> evaluatedParameters;

  private:
    void InitializePopulation(usize NP, usize N, ObjectiveFunction obj, const std::vector<f64>& LB, const std::vector<f64>& UB);
    void InitializeBestEntity();
    void InitializeOffspring(usize nParents);

    bool mConsoleOutput = true;
  };

  void CheckObjectiveFunctionNormality(ObjectiveFunction obj);
  void CheckValidationFunctionNormality(ValidationFunction valid);
  void CheckBounds();
  void CheckParameters();
  usize GetNumberOfParents();
  void InitializeOutputs(ValidationFunction valid);
  void UninitializeOutputs(const Population& population, TerminationReason reason);
  void UpdateOutputs(usize generation, const Population& population, ValidationFunction valid);
  TerminationReason CheckTerminationCriterions(const Population& population, usize generation);
  std::string GetOutputFileString(usize generation, const std::vector<f64>& bestEntity, f64 bestFitness);
  static const char* GetMutationStrategyString(MutationStrategy strategy);
  static const char* GetCrossoverStrategyString(CrossoverStrategy strategy);
  static f64 averageVectorDistance(std::vector<f64>& vec1, std::vector<f64>& vec2, std::vector<f64>& boundsRange);
  static bool isDistinct(usize inpindex, std::vector<usize>& indices, usize currindex);

  f64 mAbsoluteDifferenceThreshold = 1e-10;
  f64 mRelativeDifferenceThreshold = 0.9;
  usize mRelativeDifferenceGenerationsOverThresholdThreshold = 10;
};
