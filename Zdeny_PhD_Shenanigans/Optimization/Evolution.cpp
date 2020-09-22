#include "stdafx.h"
#include "Evolution.h"
#include "Plot/Plot1D.h"

Evolution::Evolution(int N) : OptimizationAlgorithm(N) { mNP = N * mINPm; };

OptimizationAlgorithm::OptimizationResult Evolution::Optimize(ObjectiveFunction f)
{
  LOG_INFO("Evolution optimization started");

  if (!CheckObjectiveFunctionNormality(f))
    return {};

  Population population;
  if (!population.Initialize(mNP, N, f, mLB, mUB, GetNumberOfParents()))
    return {};

  population.UpdateFunctionEvaluations(mNP);

  InitializeOutputs(population);

  bool terminate = false;
  auto reason = NotTerminated;
  int gen = 0;

  // run main evolution cycle
  LOG_INFO("Running evolution...");
  while (!terminate)
  {
    gen++;
#pragma omp parallel for
    for (int eid = 0; eid < mNP; eid++)
    {
      population.UpdateDistinctParents(eid, mNP);
      population.UpdateCrossoverParameters(eid, N, mCrossStrat, mCR);
      population.UpdateOffspring(eid, N, mMutStrat, f, mF, mLB, mUB);
    }
    population.PerformSelection(mNP);
    population.UpdateBestEntity(mNP);
    population.UpdateFitnessHistories(mNP, mHistorySize, mStopCrit, mHistoryImprovTresholdPercent);
    population.UpdateFunctionEvaluations(mNP);

    UpdateOutputs(gen, population);
    CheckTerminationCriterions(population, gen, terminate, reason);

    if (terminate)
      break;
  }

  if (mFileOutput)
    mOutputFile << "Evolution ended.\n" << std::endl;

  LOG_INFO("Evolution terminated: {}", GetTerminationReasonString(reason));
  LOG_INFO("Evolution result: {} ({})", population.bestEntity.params, population.bestEntity.fitness);
  LOG_INFO("Evolution optimization ended");
  return {population.bestEntity.params, reason};
}

void Evolution::SetFileOutput(const std::string &path)
{
  mOutputFilePath = path;
  mFileOutput = true;
}

void Evolution::InitializeOutputs(const Population &population)
{
  LOG_INFO("Initializing outputs...");

  if (mFileOutput)
  {
    mOutputFile.open(mOutputFilePath, std::ios::out);
    mOutputFile << "Evolution started." << std::endl;
    mOutputFile << GetOutputFileString(0, population.bestEntity.params, population.bestEntity.fitness);
  }

  if (mPlotOutput)
  {
    Plot1D::Reset("Evolution");
    Plot1D::plot(0, population.bestEntity.fitness, log(population.bestEntity.fitness), "Evolution", "gen", "populationFitness", "log populationFitness");
  }

  LOG_SUCC("Outputs initialized");
}

bool Evolution::CheckObjectiveFunctionNormality(ObjectiveFunction f)
{
  LOG_INFO("Checking objective function normality...");
  auto arg = 0.5 * (mLB + mUB);
  auto result1 = f(arg);
  auto result2 = f(arg);

  if (!isfinite(result1))
  {
    LOG_ERROR("Objective function is not finite");
    return false;
  }

  if (result1 != result2)
  {
    LOG_ERROR("Objective function is not consistent");
    return false;
  }

  LOG_SUCC("Objective function is normal");
  return true;
}

void Evolution::UpdateOutputs(int gen, const Population &population)
{
  if (mFileOutput)
    mOutputFile << GetOutputFileString(gen, population.bestEntity.params, population.bestEntity.fitness);

  if (mPlotOutput)
    Plot1D::plot(gen, population.bestEntity.fitness, log(population.bestEntity.fitness), "Evolution", "gen", "populationFitness", "log populationFitness");

  LOG_SUCC("Gen {} best entity: {} ({:.5f}), CBI = {:.1f}%, AHI = {:.1f}%", gen, population.bestEntity.params, population.bestEntity.fitness, (population.previousBestFitness - population.currentBestFitness) / population.previousBestFitness * 100, population.improvement * 100);
}

void Evolution::CheckTerminationCriterions(const Population &population, int generation, bool &terminate, TerminationReason &reason)
{
  if (population.bestEntity.fitness < optimalFitness) // populationFitness goal reached
  {
    terminate = true;
    reason = OptimalFitnessReached;
    return;
  }

  if (generation == maxGen) // maximum gen reached
  {
    terminate = true;
    reason = MaximumGenerationsReached;
  }

  if (population.functionEvaluations >= maxFunEvals) // maximum function evaluations exhausted
  {
    terminate = true;
    reason = MaximumFunctionEvaluationsReached;
  }

  if (population.constantHistory) // no entity improved last (mHistorySize) generations
  {
    terminate = true;
    reason = NoImprovementReached;
  }
}

std::string Evolution::GetOutputFileString(int gen, const std::vector<double> &bestEntity, double bestFitness)
{
  std::string value;
  value += "Gen " + to_string(gen);
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

Evolution::Population::Population() {}

bool Evolution::Population::Initialize(int NP, int N, ObjectiveFunction f, const std::vector<double> &LB, const std::vector<double> &UB, int nParents)
{
  try
  {
    InitializePopulation(NP, N, f, LB, UB);
    InitializeOffspring(NP, N, f, nParents);
    InitializeBestEntity(NP, N);
    functionEvaluations = 0;
    previousBestFitness = Constants::Inf;
    currentBestFitness = Constants::Inf;
    improvement = 0;
    constantHistory = false;
    return true;
  }
  catch (...)
  {
    return false;
  }
}

void Evolution::Population::UpdateDistinctParents(int eid, int NP) { offspring[eid].UpdateDistinctParents(eid, NP); }

void Evolution::Population::UpdateCrossoverParameters(int eid, int N, CrossoverStrategy crossoverStrategy, double CR) { offspring[eid].UpdateCrossoverParameters(N, crossoverStrategy, CR); }

void Evolution::Population::UpdateOffspring(int eid, int N, MutationStrategy mutationStrategy, ObjectiveFunction f, double F, const std::vector<double> &LB, const std::vector<double> &UB)
{
  auto &newoffspring = offspring[eid];
  newoffspring.params = population[eid].params;
  for (int pid = 0; pid < N; pid++)
  {
    if (newoffspring.crossoverParameters[pid])
    {
      switch (mutationStrategy)
      {
      case MutationStrategy::RAND1:
        newoffspring.params[pid] = population[newoffspring.parentIndices[0]].params[pid] + F * (population[newoffspring.parentIndices[1]].params[pid] - population[newoffspring.parentIndices[2]].params[pid]);
        break;
      case MutationStrategy::BEST1:
        newoffspring.params[pid] = bestEntity.params[pid] + F * (population[newoffspring.parentIndices[0]].params[pid] - population[newoffspring.parentIndices[1]].params[pid]);
        break;
      case MutationStrategy::RAND2:
        newoffspring.params[pid] = population[newoffspring.parentIndices[0]].params[pid] + F * (population[newoffspring.parentIndices[1]].params[pid] - population[newoffspring.parentIndices[2]].params[pid]) + F * (population[newoffspring.parentIndices[3]].params[pid] - population[newoffspring.parentIndices[4]].params[pid]);
        break;
      case MutationStrategy::BEST2:
        newoffspring.params[pid] = bestEntity.params[pid] + F * (population[newoffspring.parentIndices[0]].params[pid] - population[newoffspring.parentIndices[1]].params[pid]) + F * (population[newoffspring.parentIndices[2]].params[pid] - population[newoffspring.parentIndices[3]].params[pid]);
        break;
      }
    }
    // check for boundaries, effectively clamp
    newoffspring.params[pid] = clampSmooth(newoffspring.params[pid], population[eid].params[pid], LB[pid], UB[pid]);
    newoffspring.fitness = f(newoffspring.params);
  }
}

void Evolution::Population::PerformSelection(int NP)
{
  for (int eid = 0; eid < NP; ++eid)
  {
    if (offspring[eid].fitness <= population[eid].fitness)
    {
      population[eid].params = offspring[eid].params;
      population[eid].fitness = offspring[eid].fitness;
    }
  }
}

void Evolution::Population::UpdateFunctionEvaluations(int NP) { functionEvaluations += NP; }

void Evolution::Population::UpdateBestEntity(int NP)
{
  for (int eid = 0; eid < NP; ++eid)
    if (population[eid].fitness <= bestEntity.fitness)
      bestEntity = population[eid];

  previousBestFitness = currentBestFitness;
  currentBestFitness = bestEntity.fitness;
}

void Evolution::Population::UpdateFitnessHistories(int NP, int nHistories, StoppingCriterion stoppingCriterion, double improvThreshold)
{
  // fill history ques for all entities - termination criterion
  constantHistory = true; // assume history is constant
  improvement = 0;
  for (int eid = 0; eid < NP; ++eid)
  {
    if (population[eid].fitnessHistory.size() == nHistories)
    {
      population[eid].fitnessHistory.pop();                         // remove first element - keep que size constant
      population[eid].fitnessHistory.push(population[eid].fitness); // insert at the end
      if (stoppingCriterion == StoppingCriterion::ALLIMP)           // populationFitness improved less than x% for all entities
        if (abs(population[eid].fitnessHistory.front() - population[eid].fitnessHistory.back()) / abs(population[eid].fitnessHistory.front()) > improvThreshold / 100)
          constantHistory = false;
    }
    else
    {
      population[eid].fitnessHistory.push(population[eid].fitness); // insert at the end
      constantHistory = false;                                      // too early to stop, go on
    }
    if (population[eid].fitnessHistory.size() > 2)
      improvement += abs(population[eid].fitnessHistory.front()) == 0 ? 0 : abs(population[eid].fitnessHistory.front() - population[eid].fitnessHistory.back()) / abs(population[eid].fitnessHistory.front());
  }
  improvement /= NP;
  if (stoppingCriterion == StoppingCriterion::AVGIMP) // average populationFitness improved less than x%
    if (100 * improvement > improvThreshold)
      constantHistory = false;
}

void Evolution::Population::InitializePopulation(int NP, int N, ObjectiveFunction f, const std::vector<double> &LB, const std::vector<double> &UB)
{
  LOG_INFO("Creating initial population within bounds...");
  population = zerovect(NP, Entity(N));
  std::vector<double> RB = UB - LB;
  const double initialMinAvgDist = 0.5;
  const int distincEntityMaxTrials = 10;
  double minAvgDist = initialMinAvgDist;

  for (int eid = 0; eid < NP; eid++)
  {
    int distinctEntityTrials = 0;
    bool distinctEntity = false; // entities may not be too close together
    while (!distinctEntity)      // loop until they are distinct enough
    {
      distinctEntity = true; // assume entity is distinct

      for (int pid = 0; pid < N; pid++) // generate initial entity
        population[eid].params[pid] = randr(LB[pid], UB[pid]);

      if (distincEntityMaxTrials < 1)
        break;

      for (int eidx2 = 0; eidx2 < eid; eidx2++) // check distance to all other entities
      {
        double avgDist = averageVectorDistance(population[eid].params, population[eidx2].params, RB); // calculate how distinct the entity is to another entity
        if (avgDist < minAvgDist)                                                                     // check if entity is distinct
        {
          distinctEntity = false;
          break; // needs to be distinct from all entities
        }
      }

      distinctEntityTrials++;

      if (distinctEntityTrials >= distincEntityMaxTrials)
      {
        minAvgDist *= 0.8;
        distinctEntityTrials = 0;
      }
    }
  }
  LOG_SUCC("Initial population created");

  LOG_INFO("Evaluating initial population...");
#pragma omp parallel for
  for (int eid = 0; eid < NP; eid++)
    population[eid].fitness = f(population[eid].params);
  LOG_SUCC("Initial population evaluated");
}

void Evolution::Population::InitializeOffspring(int NP, int N, ObjectiveFunction f, int nParents)
{
  LOG_INFO("Creating initial offspring...");
  offspring = zerovect(NP, Offspring(N, nParents));
  for (int eid = 0; eid < NP; eid++)
  {
    offspring[eid].params = population[eid].params;
    offspring[eid].fitness = population[eid].fitness;
  }
  LOG_SUCC("Initial offspring created");
}

void Evolution::Population::InitializeBestEntity(int NP, int N)
{
  LOG_INFO("Searching for best entity in the initial population...");
  bestEntity = Entity(N);
  for (int eid = 0; eid < NP; eid++)
    if (population[eid].fitness <= bestEntity.fitness)
      bestEntity = population[eid];

  LOG_SUCC("Initial population best entity: {} ({:.3f})", bestEntity.params, bestEntity.fitness);
}

Evolution::Entity::Entity() {}

Evolution::Entity::Entity(int N)
{
  params.resize(N);
  fitness = Constants::Inf;
}

Evolution::Offspring::Offspring() {}

Evolution::Offspring::Offspring(int N, int nParents)
{
  params.resize(N);
  parentIndices.resize(nParents);
  crossoverParameters.resize(N);
  fitness = Constants::Inf;
}

void Evolution::Offspring::UpdateDistinctParents(int eid, int NP)
{
  for (auto &idx : parentIndices)
  {
    int idxTst = rand() % NP;
    while (!isDistinct(idxTst, parentIndices, eid))
      idxTst = rand() % NP;
    idx = idxTst;
  }
}

void Evolution::Offspring::UpdateCrossoverParameters(int N, CrossoverStrategy crossoverStrategy, double CR)
{
  crossoverParameters = zerovect(N, false);

  switch (crossoverStrategy)
  {
  case CrossoverStrategy::BIN:
  {
    int definite = rand() % N; // at least one param undergoes crossover
    for (int pid = 0; pid < N; pid++)
    {
      double random = rand01();
      if (random < CR || pid == definite)
        crossoverParameters[pid] = true;
    }
    return;
  }
  case CrossoverStrategy::EXP:
  {
    int L = 0;
    do
      L++;
    while ((rand01() < CR) && (L < N)); // at least one param undergoes crossover
    int pid = rand() % N;
    for (int i = 0; i < L; i++)
    {
      crossoverParameters[pid] = true;
      pid++;
      pid %= N;
    }
    return;
  }
  }
}
