#pragma once
#include "Optimization.hpp"

class Evolution : public OptimizationAlgorithm
{
public:
  enum MutationStrategy : uint8_t
  {
    RAND1,
    RAND2,
    BEST1,
    BEST2,
    MutationStrategyCount
  };

  enum CrossoverStrategy : uint8_t
  {
    BIN,
    EXP,
    CrossoverStrategyCount
  };

  enum MetaObjectiveFunctionType : uint8_t
  {
    ObjectiveFunctionValue
  };

  explicit Evolution(size_t N_, const std::string& optname = "default");

  OptimizationResult Optimize(const ObjectiveFunction& obj, const std::optional<ObjectiveFunction>& valid = std::nullopt) override;

  void MetaOptimize(ObjectiveFunction obj, MetaObjectiveFunctionType metaObjType = ObjectiveFunctionValue, size_t runsPerObj = 3, size_t maxFunEvals = 10000,
      double optimalFitness = -std::numeric_limits<double>::max());

  size_t mNP = 4;
  double mF = 0.65;
  double mCR = 0.90;
  MutationStrategy mMutStrat = RAND1;
  CrossoverStrategy mCrossStrat = BIN;

private:
  struct Entity
  {
    Entity() = default;
    explicit Entity(size_t N);

    std::vector<double> params;
    double fitness;
  };

  struct Offspring
  {
    Offspring() = default;
    Offspring(size_t N, size_t nParents);
    void UpdateDistinctParents(size_t eid, size_t NP);
    void UpdateCrossoverParameters(CrossoverStrategy crossoverStrategy, double CR);

    std::vector<double> params;
    double fitness;
    std::vector<size_t> parentIndices;
    std::vector<bool> crossoverParameters;
  };

  struct Population
  {
    Population(size_t NP, size_t N, ObjectiveFunction obj, const std::vector<double>& LB, const std::vector<double>& UB, size_t nParents, bool consoleOutput, bool saveProgress);
    void UpdateDistinctParents(size_t eid);
    void UpdateCrossoverParameters(size_t eid, CrossoverStrategy crossoverStrategy, double CR);
    void UpdateOffspring(size_t eid, MutationStrategy mutationStrategy, ObjectiveFunction obj, double F, const std::vector<double>& LB, const std::vector<double>& UB);
    void PerformSelection();
    void UpdateBestEntity();
    void UpdateTerminationCriterions(double relativeDifferenceThreshold);
    void UpdateProgress();

    std::vector<Entity> entities;
    std::vector<Offspring> offspring;
    Entity bestEntity;
    size_t functionEvaluations;
    double averageFitness;
    double previousFitness;
    double absoluteDifference;
    double relativeDifference;
    size_t relativeDifferenceGenerationsOverThreshold;
    bool mSaveProgress;
    std::vector<double> bestFitnessProgress;
    std::vector<std::vector<double>> bestParametersProgress;
    std::vector<std::vector<double>> evaluatedParameters;

  private:
    void InitializePopulation(size_t NP, size_t N, ObjectiveFunction obj, const std::vector<double>& LB, const std::vector<double>& UB);
    void InitializeBestEntity();
    void InitializeOffspring(size_t nParents);

    bool mConsoleOutput = true;
  };

  void CheckObjectiveFunctionNormality(ObjectiveFunction obj) const;
  void CheckBounds();
  void CheckParameters() const;
  size_t GetNumberOfParents() const;
  void UninitializeOutputs(const Population& population, TerminationReason reason, size_t generation);
  void UpdateOutputs(size_t generation, const Population& population, const std::optional<ObjectiveFunction>& valid);
  TerminationReason CheckTerminationCriterions(const Population& population, size_t generation) const;
  std::string GetOutputString(size_t generation, const Population& population);
  static const char* GetMutationStrategyString(MutationStrategy strategy);
  static const char* GetCrossoverStrategyString(CrossoverStrategy strategy);
  static double averageVectorDistance(const std::vector<double>& vec1, const std::vector<double>& vec2, const std::vector<double>& boundsRange);

  double mAbsoluteDifferenceThreshold = 1e-10;
  double mRelativeDifferenceThreshold = 0.9;
  size_t mRelativeDifferenceGenerationsOverThresholdThreshold = 10;
};
