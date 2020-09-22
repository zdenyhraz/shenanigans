#include "stdafx.h"
#include "Evolution.h"
#include "Plot/Plot1D.h"

Evolution::Evolution(int N) : OptimizationAlgorithm(N) { mNP = N * mINPm; };

std::tuple<std::vector<double>, OptimizationAlgorithm::TerminationReason> Evolution::optimize(const std::function<double(const std::vector<double> &)> &f)
{
  LOG_INFO("Evolution optimization started");

  if (!InitializeOutputs())
    return {};

  if (!CheckObjectiveFunctionNormality(f))
    return {};

  // establish all internals
  TerminationReason terminationReason = NotTerminated;
  std::vector<std::queue<double>> populationHistory(mNP);
  bool historyConstant = false;
  bool terminate = false;
  int numberOfParents = GetNumberOfParents();
  int functionEvaluations = 0;
  auto parentIndices = zerovect2(mNP, numberOfParents, 0);
  auto crossoverParameters = zerovect2(mNP, N, false);
  double averageImprovement = 0;
  double fitnessPrevious = Constants::Inf;
  double fitnessCurrent = Constants::Inf;
  auto population = InitializePopulation();
  auto populationFitness = InitializeFitness(population, f);
  auto [bestEntity, bestFitness] = InitializeBestEntity(population, populationFitness);
  auto offspring = population;
  auto offspringFitness = populationFitness;
  UpdateFunctionEvaluations(functionEvaluations);

  // run main evolution cycle
  LOG_INFO("Running evolution...");
  for (int generation = 1; generation < 1e10; generation++)
  {
#pragma omp parallel for
    for (int indexEntity = 0; indexEntity < mNP; indexEntity++)
    {
      // crossover
      CalculateDistinctParents(indexEntity, parentIndices[indexEntity]);
      CalculateCrossoverParameters(crossoverParameters[indexEntity]);
      // mutation
      CalculateOffspring(population, bestEntity, parentIndices[indexEntity], crossoverParameters[indexEntity], indexEntity, offspring[indexEntity]);
      CalculateOffspringFitness(offspring[indexEntity], f, offspringFitness[indexEntity]);
      // selection
      CalculateSelection(offspring[indexEntity], offspringFitness[indexEntity], population[indexEntity], populationFitness[indexEntity]);
    }
    UpdateFunctionEvaluations(functionEvaluations);
    CalculateBestEntity(population, populationFitness, bestEntity, bestFitness);
    UpdateOutputs(generation, bestEntity, bestFitness, averageImprovement, fitnessPrevious, fitnessCurrent);
    UpdateHistories(populationFitness, populationHistory, averageImprovement, historyConstant);
    std::tie(terminate, terminationReason) = CheckTerminationCriterions(bestFitness, generation, functionEvaluations, historyConstant);

    if (terminate)
      break;
  }

  if (mFileOutput)
    mOutputFile << "Evolution ended.\n" << std::endl;

  LOG_INFO("Evolution terminated: {}", GetTerminationReasonString(terminationReason));
  LOG_INFO("Evolution result: {} ({})", bestEntity, bestFitness);
  LOG_INFO("Evolution optimization ended");
  return {bestEntity, terminationReason};
}

void Evolution::SetFileOutput(const std::string &path)
{
  mOutputFilePath = path;
  mFileOutput = true;
}

bool Evolution::InitializeOutputs()
{
  try
  {
    if (mFileOutput)
    {
      mOutputFile.open(mOutputFilePath, std::ios::out);
      mOutputFile << "Evolution started." << std::endl;
    }

    if (mPlotOutput)
      Plot1D::Reset("Evolution");
  }
  catch (...)
  {
    return false;
  }
  return true;
}

std::vector<std::vector<double>> Evolution::InitializePopulation()
{
  LOG_INFO("Creating initial population within bounds ... ");
  std::vector<std::vector<double>> population(mNP, zerovect(N, 0.));
  std::vector<double> boundsRange = upperBounds - lowerBounds;
  const double initialMinAvgDist = 0.5;
  double minAvgDist = initialMinAvgDist;
  for (int indexEntity = 0; indexEntity < mNP; indexEntity++)
  {
    int distinctEntityTrials = 0;
    bool distinctEntity = false; // entities may not be too close together
    while (!distinctEntity)      // loop until they are distinct enough
    {
      distinctEntity = true; // assume entity is distinct

      for (int indexParam = 0; indexParam < N; indexParam++) // generate initial entity
        population[indexEntity][indexParam] = randr(lowerBounds[indexParam], upperBounds[indexParam]);

      if (!mDistincEntityMaxTrials)
        break;

      for (int indexEntity2 = 0; indexEntity2 < indexEntity; indexEntity2++) // check distance to all other entities
      {
        double avgDist = averageVectorDistance(population[indexEntity], population[indexEntity2], boundsRange); // calculate how distinct the entity is to another entity
        if (avgDist < minAvgDist)                                                                               // check if entity is distinct
        {
          distinctEntity = false;
          break; // needs to be distinct from all entities
        }
      }

      distinctEntityTrials++;

      if (distinctEntityTrials >= mDistincEntityMaxTrials)
      {
        minAvgDist *= 0.8;
        distinctEntityTrials = 0;
      }
    }
  }
  LOG_SUCC("Initial population created");
  return population;
}

std::vector<double> Evolution::InitializeFitness(const std::vector<std::vector<double>> &population, const std::function<double(const std::vector<double> &)> &f)
{
  LOG_INFO("Evaluating initial population...");
  std::vector<double> populationFitness = zerovect(mNP, 0.);

#pragma omp parallel for
  for (int indexEntity = 0; indexEntity < mNP; indexEntity++)
    populationFitness[indexEntity] = f(population[indexEntity]);

  LOG_SUCC("Initial population evaluated");
  return populationFitness;
}

std::pair<std::vector<double>, double> Evolution::InitializeBestEntity(const std::vector<std::vector<double>> &population, const std::vector<double> &populationFitness)
{
  std::vector<double> bestEntity = zerovect(N, 0.);
  double bestFitness = Constants::Inf;
  for (int indexEntity = 0; indexEntity < mNP; indexEntity++)
  {
    if (populationFitness[indexEntity] <= bestFitness)
    {
      bestEntity = population[indexEntity];
      bestFitness = populationFitness[indexEntity];
    }
  }

  if (mFileOutput)
    mOutputFile << GetOutputFileString(0, bestEntity, bestFitness);

  if (mPlotOutput)
    Plot1D::plot(0, bestFitness, log(bestFitness), "Evolution", "generation", "populationFitness", "log populationFitness");

  LOG_INFO("Initial population best entity: {} ({:.3f})", bestEntity, bestFitness);
  return {bestEntity, bestFitness};
}

bool Evolution::CheckObjectiveFunctionNormality(const std::function<double(const std::vector<double> &)> &f)
{
  LOG_INFO("Checking objective function normality...");
  if (!isfinite(f(0.5 * (lowerBounds + upperBounds))))
  {
    LOG_ERROR("Objective function is not normal!");
    return false;
  }

  LOG_SUCC("Objective function is normal");
  return true;
}

void Evolution::CalculateDistinctParents(int entityIndex, std::vector<int> &parentIndices)
{
  for (auto &idx : parentIndices)
  {
    int idxTst = rand() % mNP;
    while (!isDistinct(idxTst, parentIndices, entityIndex))
      idxTst = rand() % mNP;
    idx = idxTst;
  }
}

void Evolution::CalculateCrossoverParameters(std::vector<bool> &crossoverParameters)
{
  crossoverParameters = zerovect(N, false);

  switch (mCrossStrat)
  {
  case CrossoverStrategy::BIN:
  {
    int definite = rand() % N; // at least one param undergoes crossover
    for (int indexParam = 0; indexParam < N; indexParam++)
    {
      double random = rand01();
      if (random < mCR || indexParam == definite)
        crossoverParameters[indexParam] = true;
    }
    return;
  }
  case CrossoverStrategy::EXP:
  {
    int L = 0;
    do
      L++;
    while ((rand01() < mCR) && (L < N)); // at least one param undergoes crossover
    int indexParam = rand() % N;
    for (int i = 0; i < L; i++)
    {
      crossoverParameters[indexParam] = true;
      indexParam++;
      indexParam %= N;
    }
    return;
  }
  }
}

void Evolution::CalculateOffspring(const std::vector<std::vector<double>> &population, const std::vector<double> &bestEntity, const std::vector<int> &parentIndices, const std::vector<bool> &crossoverParameters, int indexEntity, std::vector<double> &offspring)
{
  offspring = population[indexEntity];
  for (int indexParam = 0; indexParam < N; indexParam++)
  {
    if (crossoverParameters[indexParam])
    {
      switch (mMutStrat)
      {
      case MutationStrategy::RAND1:
        offspring[indexParam] = population[parentIndices[0]][indexParam] + mF * (population[parentIndices[1]][indexParam] - population[parentIndices[2]][indexParam]);
        break;
      case MutationStrategy::BEST1:
        offspring[indexParam] = bestEntity[indexParam] + mF * (population[parentIndices[0]][indexParam] - population[parentIndices[1]][indexParam]);
        break;
      case MutationStrategy::RAND2:
        offspring[indexParam] = population[parentIndices[0]][indexParam] + mF * (population[parentIndices[1]][indexParam] - population[parentIndices[2]][indexParam]) + mF * (population[parentIndices[3]][indexParam] - population[parentIndices[4]][indexParam]);
        break;
      case MutationStrategy::BEST2:
        offspring[indexParam] = bestEntity[indexParam] + mF * (population[parentIndices[0]][indexParam] - population[parentIndices[1]][indexParam]) + mF * (population[parentIndices[2]][indexParam] - population[parentIndices[3]][indexParam]);
        break;
      }
    }
    // check for boundaries, effectively clamp
    offspring[indexParam] = clampSmooth(offspring[indexParam], population[indexEntity][indexParam], lowerBounds[indexParam], upperBounds[indexParam]);
  }
}

void Evolution::CalculateOffspringFitness(const std::vector<double> &offspring, const std::function<double(const std::vector<double> &)> &f, double &offspringFitness) { offspringFitness = f(offspring); }

void Evolution::CalculateSelection(const std::vector<double> &offspring, double offspringFitness, std::vector<double> &population, double &populationFitness)
{
  if (offspringFitness <= populationFitness)
  {
    population = offspring;
    populationFitness = offspringFitness;
  }
}

void Evolution::UpdateFunctionEvaluations(int &functionEvaluations) { functionEvaluations += mNP; }

void Evolution::CalculateBestEntity(const std::vector<std::vector<double>> &population, const std::vector<double> &populationFitness, std::vector<double> &bestEntity, double &bestFitness)
{
  for (int indexEntity = 0; indexEntity < mNP; indexEntity++)
  {
    if (populationFitness[indexEntity] <= bestFitness)
    {
      bestEntity = population[indexEntity];
      bestFitness = populationFitness[indexEntity];
    }
  }
}

void Evolution::UpdateOutputs(int generation, const std::vector<double> &bestEntity, double bestFitness, double averageImprovement, double &fitnessPrevious, double &fitnessCurrent)
{
  if (mFileOutput)
    mOutputFile << GetOutputFileString(generation, bestEntity, bestFitness);

  if (mPlotOutput)
    Plot1D::plot(generation, bestFitness, log(bestFitness), "Evolution", "generation", "populationFitness", "log populationFitness");

  fitnessPrevious = fitnessCurrent;
  fitnessCurrent = bestFitness;
  LOG_SUCC("Gen {} best entity: {} ({:.5f}), CBI = {:.1f}%, AHI = {:.1f}%", generation, bestEntity, bestFitness, (fitnessPrevious - fitnessCurrent) / fitnessPrevious * 100, averageImprovement * 100);
}

void Evolution::UpdateHistories(const std::vector<double> &populationFitness, std::vector<std::queue<double>> &populationHistory, double &averageImprovement, bool &historyConstant)
{
  // fill history ques for all entities - termination criterion
  historyConstant = true; // assume history is constant
  averageImprovement = 0;
  for (int indexEntity = 0; indexEntity < mNP; indexEntity++)
  {
    if (populationHistory[indexEntity].size() == mHistorySize)
    {
      populationHistory[indexEntity].pop();                                // remove first element - keep que size constant
      populationHistory[indexEntity].push(populationFitness[indexEntity]); // insert at the end
      if (mStopCrit == StoppingCriterion::ALLIMP)                          // populationFitness improved less than x% for all entities
        if (abs(populationHistory[indexEntity].front() - populationHistory[indexEntity].back()) / abs(populationHistory[indexEntity].front()) > mHistoryImprovTresholdPercent / 100)
          historyConstant = false;
    }
    else
    {
      populationHistory[indexEntity].push(populationFitness[indexEntity]); // insert at the end
      historyConstant = false;                                             // too early to stop, go on
    }
    if (populationHistory[indexEntity].size() > 2)
      averageImprovement += abs(populationHistory[indexEntity].front()) == 0 ? 0 : abs(populationHistory[indexEntity].front() - populationHistory[indexEntity].back()) / abs(populationHistory[indexEntity].front());
  }
  averageImprovement /= mNP;
  if (mStopCrit == StoppingCriterion::AVGIMP) // average populationFitness improved less than x%
    if (100 * averageImprovement > mHistoryImprovTresholdPercent)
      historyConstant = false;
}

std::pair<bool, OptimizationAlgorithm::TerminationReason> Evolution::CheckTerminationCriterions(double bestFitness, int generation, int functionEvaluations, bool historyConstant)
{
  if (bestFitness < optimalFitness) // populationFitness goal reached
    return {true, OptimalFitnessReached};

  if (generation == maxGen) // maximum generation reached
    return {true, MaximumGenerationsReached};

  if (functionEvaluations >= maxFunEvals) // maximum function evaluations exhausted
    return {true, MaximumFunctionEvaluationsReached};

  if (historyConstant) // no entity improved last (mHistorySize) generations
    return {true, NoImprovementReached};

  return {false, NotTerminated};
}

std::string Evolution::GetOutputFileString(int generation, const std::vector<double> &bestEntity, double bestFitness)
{
  std::string value;
  value += "Gen " + to_string(generation);
  value += " best: [";
  for (int i = 0; i < bestEntity.size(); ++i)
  {
    if (mParameterNames.size() > i)
    {
      if (i != bestEntity.size() - 1)
        value += mParameterNames[i] + ": " + to_string(bestEntity[i]) + ", ";
      else
        value += mParameterNames[i] + ": " + to_string(bestEntity[i]) + "]";
    }
    else
    {
      if (i != bestEntity.size() - 1)
        value += "param" + to_string(i) + ": " + to_string(bestEntity[i]) + ", ";
      else
        value += "param" + to_string(i) + ": " + to_string(bestEntity[i]) + "]";
    }
  }
  value += " (Obj: " + to_string(bestFitness) + ")\n";
  return value;
}

int Evolution::GetNumberOfParents()
{
  switch (mMutStrat)
  {
  case MutationStrategy::RAND1:
    return 3;
  case MutationStrategy::BEST1:
    return 2;
  case MutationStrategy::RAND2:
    return 5;
  case MutationStrategy::BEST2:
    return 4;
  }
  return 3;
}
