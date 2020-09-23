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

  InitializeOutputs(population);

  bool terminate = false;
  auto reason = NotTerminated;
  int gen = 0;

  while (!terminate)
  {
    gen++;
#pragma omp parallel for
    for (int eid = 0; eid < mNP; eid++)
    {
      population.UpdateDistinctParents(eid);
      population.UpdateCrossoverParameters(eid, mCrossStrat, mCR);
      population.UpdateOffspring(eid, mMutStrat, f, mF, mLB, mUB);
    }
    population.PerformSelection();
    population.UpdateBestEntity();
    population.UpdateFitnessHistories(mHistorySize, mStopCrit, mHistoryImprovTresholdPercent);
    population.UpdateOffspringFunctionEvaluations();

    UpdateOutputs(gen, population);
    CheckTerminationCriterions(population, gen, terminate, reason);

    if (terminate)
      break;
  }

  UninitializeOutputs(population, reason);
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
    mOutputFile << "Running evolution..." << std::endl;
    mOutputFile << GetOutputFileString(0, population.bestEntity.params, population.bestEntity.fitness);
  }

  if (mPlotOutput)
  {
    Plot1D::Reset("Evolution");
    Plot1D::plot(0, population.bestEntity.fitness, log(population.bestEntity.fitness), "Evolution", "gen", "populationFitness", "log populationFitness");
  }

  LOG_SUCC("Outputs initialized");
  LOG_INFO("Running evolution...");
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
  else
    LOG_DEBUG("Objective function is finite");

  if (result1 != result2)
  {
    LOG_ERROR("Objective function is not consistent");
    return false;
  }
  else
    LOG_DEBUG("Objective function is consistent");

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

void Evolution::UninitializeOutputs(const Population &population, TerminationReason reason)
{
  if (mFileOutput)
    mOutputFile << "Evolution optimization ended\n" << std::endl;

  LOG_INFO("Evolution terminated: {}", GetTerminationReasonString(reason));
  LOG_INFO("Evolution result: {} ({})", population.bestEntity.params, population.bestEntity.fitness);
  LOG_INFO("Evolution optimization ended");
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
    InitializeBestEntity(NP, N);
    InitializeOffspring(NP, N, f, nParents);
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

void Evolution::Population::UpdateDistinctParents(int eid) { offspring[eid].UpdateDistinctParents(eid, entities.size()); }

void Evolution::Population::UpdateCrossoverParameters(int eid, CrossoverStrategy crossoverStrategy, double CR) { offspring[eid].UpdateCrossoverParameters(crossoverStrategy, CR); }

void Evolution::Population::UpdateOffspring(int eid, MutationStrategy mutationStrategy, ObjectiveFunction f, double F, const std::vector<double> &LB, const std::vector<double> &UB)
{
  auto &newoffspring = offspring[eid];
  newoffspring.params = entities[eid].params;
  for (int pid = 0; pid < newoffspring.params.size(); pid++)
  {
    if (newoffspring.crossoverParameters[pid])
    {
      switch (mutationStrategy)
      {
      case MutationStrategy::RAND1:
        newoffspring.params[pid] = entities[newoffspring.parentIndices[0]].params[pid] + F * (entities[newoffspring.parentIndices[1]].params[pid] - entities[newoffspring.parentIndices[2]].params[pid]);
        break;
      case MutationStrategy::BEST1:
        newoffspring.params[pid] = bestEntity.params[pid] + F * (entities[newoffspring.parentIndices[0]].params[pid] - entities[newoffspring.parentIndices[1]].params[pid]);
        break;
      case MutationStrategy::RAND2:
        newoffspring.params[pid] = entities[newoffspring.parentIndices[0]].params[pid] + F * (entities[newoffspring.parentIndices[1]].params[pid] - entities[newoffspring.parentIndices[2]].params[pid]) + F * (entities[newoffspring.parentIndices[3]].params[pid] - entities[newoffspring.parentIndices[4]].params[pid]);
        break;
      case MutationStrategy::BEST2:
        newoffspring.params[pid] = bestEntity.params[pid] + F * (entities[newoffspring.parentIndices[0]].params[pid] - entities[newoffspring.parentIndices[1]].params[pid]) + F * (entities[newoffspring.parentIndices[2]].params[pid] - entities[newoffspring.parentIndices[3]].params[pid]);
        break;
      }
    }
    // check for boundaries, effectively clamp
    newoffspring.params[pid] = clampSmooth(newoffspring.params[pid], entities[eid].params[pid], LB[pid], UB[pid]);
  }
  newoffspring.fitness = f(newoffspring.params);
}

void Evolution::Population::PerformSelection()
{
  for (int eid = 0; eid < entities.size(); ++eid)
  {
    if (offspring[eid].fitness <= entities[eid].fitness)
    {
      entities[eid].params = offspring[eid].params;
      entities[eid].fitness = offspring[eid].fitness;
    }
  }
}

void Evolution::Population::UpdatePopulationFunctionEvaluations() { functionEvaluations += entities.size(); }

void Evolution::Population::UpdateOffspringFunctionEvaluations() { functionEvaluations += offspring.size(); }

void Evolution::Population::UpdateBestEntity()
{
  for (int eid = 0; eid < entities.size(); ++eid)
    if (entities[eid].fitness <= bestEntity.fitness)
      bestEntity = entities[eid];

  previousBestFitness = currentBestFitness;
  currentBestFitness = bestEntity.fitness;
}

void Evolution::Population::UpdateFitnessHistories(int nHistories, StoppingCriterion stoppingCriterion, double improvThreshold)
{
  // fill history ques for all entities - termination criterion
  constantHistory = true; // assume history is constant
  improvement = 0;
  for (int eid = 0; eid < entities.size(); ++eid)
  {
    if (entities[eid].fitnessHistory.size() == nHistories)
    {
      entities[eid].fitnessHistory.pop();                       // remove first element - keep que size constant
      entities[eid].fitnessHistory.push(entities[eid].fitness); // insert at the end
      if (stoppingCriterion == StoppingCriterion::ALLIMP)       // populationFitness improved less than x% for all entities
        if (abs(entities[eid].fitnessHistory.front() - entities[eid].fitnessHistory.back()) / abs(entities[eid].fitnessHistory.front()) > improvThreshold / 100)
          constantHistory = false;
    }
    else
    {
      entities[eid].fitnessHistory.push(entities[eid].fitness); // insert at the end
      constantHistory = false;                                  // too early to stop, go on
    }
    if (entities[eid].fitnessHistory.size() > 2)
      improvement += abs(entities[eid].fitnessHistory.front()) == 0 ? 0 : abs(entities[eid].fitnessHistory.front() - entities[eid].fitnessHistory.back()) / abs(entities[eid].fitnessHistory.front());
  }
  improvement /= entities.size();
  if (stoppingCriterion == StoppingCriterion::AVGIMP) // average populationFitness improved less than x%
    if (100 * improvement > improvThreshold)
      constantHistory = false;
}

void Evolution::Population::InitializePopulation(int NP, int N, ObjectiveFunction f, const std::vector<double> &LB, const std::vector<double> &UB)
{
  LOG_INFO("Creating initial population within bounds...");
  entities = zerovect(NP, Entity(N));
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
        entities[eid].params[pid] = randr(LB[pid], UB[pid]);

      if (distincEntityMaxTrials < 1)
        break;

      for (int eidx2 = 0; eidx2 < eid; eidx2++) // check distance to all other entities
      {
        double avgDist = averageVectorDistance(entities[eid].params, entities[eidx2].params, RB); // calculate how distinct the entity is to another entity
        if (avgDist < minAvgDist)                                                                 // check if entity is distinct
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
    entities[eid].fitness = f(entities[eid].params);

  UpdatePopulationFunctionEvaluations();
  LOG_SUCC("Initial population evaluated");
}

void Evolution::Population::InitializeOffspring(int NP, int N, ObjectiveFunction f, int nParents)
{
  LOG_INFO("Creating initial offspring...");
  offspring = zerovect(NP, Offspring(N, nParents));
  for (int eid = 0; eid < NP; eid++)
  {
    offspring[eid].params = entities[eid].params;
    offspring[eid].fitness = entities[eid].fitness;
  }
  LOG_SUCC("Initial offspring created");
}

void Evolution::Population::InitializeBestEntity(int NP, int N)
{
  LOG_INFO("Searching for best entity in the initial population...");
  bestEntity = Entity(N);
  for (int eid = 0; eid < NP; eid++)
    if (entities[eid].fitness <= bestEntity.fitness)
      bestEntity = entities[eid];

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

void Evolution::Offspring::UpdateCrossoverParameters(CrossoverStrategy crossoverStrategy, double CR)
{
  crossoverParameters = zerovect(params.size(), false);

  switch (crossoverStrategy)
  {
  case CrossoverStrategy::BIN:
  {
    int definite = rand() % params.size(); // at least one param undergoes crossover
    for (int pid = 0; pid < params.size(); pid++)
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
    while ((rand01() < CR) && (L < params.size())); // at least one param undergoes crossover
    int pid = rand() % params.size();
    for (int i = 0; i < L; i++)
    {
      crossoverParameters[pid] = true;
      pid++;
      pid %= params.size();
    }
    return;
  }
  }
}
