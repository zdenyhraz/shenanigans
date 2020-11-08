#include "stdafx.h"
#include "Evolution.h"
#include "Plot/Plot1D.h"

Evolution::Evolution(int N, const std::string &optname) : OptimizationAlgorithm(N), mOptimizationName(optname), mNP(5.4 * N){};

OptimizationAlgorithm::OptimizationResult Evolution::Optimize(ObjectiveFunction f)
{
  Population population;

  if (!InitializeOutputs())
    return {};

  if (!CheckObjectiveFunctionNormality(f))
    return {};

  if (!CheckBounds())
    return {};

  if (!population.Initialize(mNP, N, f, mLB, mUB, GetNumberOfParents()))
    return {};

  int gen = 0;
  TerminationReason treason = NotTerminated;
  population.UpdateTerminationCriterions(mRelativeDifferenceThreshold);
  LOG_INFO("Running evolution...");
  UpdateOutputs(gen, population);

  while (!treason)
  {
    try
    {
#pragma omp parallel for
      for (int eid = 0; eid < mNP; ++eid)
      {
        population.UpdateDistinctParents(eid);
        population.UpdateCrossoverParameters(eid, mCrossStrat, mCR);
        population.UpdateOffspring(eid, mMutStrat, f, mF, mLB, mUB);
      }
      population.PerformSelection();
      population.UpdateBestEntity();
      population.UpdateOffspringFunctionEvaluations();
      population.UpdateTerminationCriterions(mRelativeDifferenceThreshold);
      gen++;

      treason = CheckTerminationCriterions(population, gen);
      UpdateOutputs(gen, population);
    }
    catch (const std::exception &e)
    {
      LOG_ERROR("Unexpected error occured during generation {}: {}", gen, e.what());
      treason = UnexpectedErrorOccured;
    }
  }

  UninitializeOutputs(population, treason);
  return population.bestEntity.params;
}

void Evolution::SetFileOutputDir(const std::string &dir)
{
  mOutputFileDir = dir;
  mFileOutput = true;
}

bool Evolution::InitializeOutputs()
{
  try
  {
    LOG_INFO("Evolution optimization started");
    LOG_INFO("Initializing outputs...");

    if (mFileOutput)
    {
      mOutputFile.open(mOutputFileDir + mOptimizationName + ".txt", std::ios::out | std::ios::app);
      mOutputFile << "Evolution optimization '" + mOptimizationName + "' started" << std::endl;
    }

    if (mPlotOutput)
    {
      Plot1D::Reset("Evolution");
      Plot1D::Reset("EvolutionDIiff");
    }

    LOG_SUCC("Outputs initialized");
    return true;
  }
  catch (const std::exception &e)
  {
    LOG_ERROR("Could not initialize outputs: {}", e.what());
    return false;
  }
}

bool Evolution::CheckObjectiveFunctionNormality(ObjectiveFunction f)
{
  LOG_INFO("Checking objective function normality...");
  try
  {
    auto arg = 0.5 * (mLB + mUB);
    auto result = f(arg);
    auto result2 = f(arg);

    if (!isfinite(result))
    {
      LOG_ERROR("Objective function is not finite");
      return false;
    }
    else
      LOG_DEBUG("Objective function is finite");

    if (result < 0)
    {
      LOG_ERROR("Objective function is not positive");
      return false;
    }
    else
      LOG_DEBUG("Objective function is positive");

    if (result != result2)
    {
      LOG_ERROR("Objective function is not consistent");
      return false;
    }
    else
      LOG_DEBUG("Objective function is consistent");

    LOG_SUCC("Objective function is normal");
    return true;
  }
  catch (const std::exception &e)
  {
    LOG_ERROR("Objective function is not normal: {}", e.what());
    return false;
  }
}

bool Evolution::CheckBounds()
{
  LOG_INFO("Checking objective function parameter bounds validity...");

  if (mLB.size() != mUB.size())
  {
    LOG_ERROR("Parameter bound sizes do not match");
    return false;
  }

  if (mLB.size() != N)
  {
    LOG_ERROR("Invalid lower parameter bound size: {} != {}", mLB.size(), N);
    return false;
  }

  if (mUB.size() != N)
  {
    LOG_ERROR("Invalid upper parameter bound size: {} != {}", mUB.size(), N);
    return false;
  }

  LOG_SUCC("Objective function parameter bounds are valid");
  return true;
}

void Evolution::UpdateOutputs(int gen, const Population &population)
{
  if (population.bestEntity.fitness < population.previousFitness)
  {
    auto message = GetOutputFileString(gen, population.bestEntity.params, population.bestEntity.fitness);

    if (mFileOutput)
      mOutputFile << message << std::endl;

    LOG_DEBUG("{}, reldif : {:.2f}, absdif : {:.2e}", message, population.relativeDifference, population.absoluteDifference);
  }

  if (mPlotOutput)
  {
    Plot1D::plot(gen, {population.bestEntity.fitness, population.averageFitness}, {log(population.bestEntity.fitness)}, "Evolution", "generation",
        "fitness", "log(fitness)", {"bestFitness", "averageFitness"}, {"log(bestFitness)"},
        {QPen(Plot::green, 2), QPen(Plot::black, 2), QPen(Plot::magenta, 2)});
    Plot1D::plot(gen, {population.absoluteDifference}, {population.relativeDifference, mRelativeDifferenceThreshold}, "EvolutionDIiff", "generation",
        "best-average absolute difference", "best-average relative difference", {"absdif"}, {"reldif", "reldif thr"},
        {QPen(Plot::black, 2), QPen(Plot::green, 2), QPen(Plot::red, 1, Qt::DotLine)});
  }
}

void Evolution::UninitializeOutputs(const Population &population, TerminationReason reason)
{
  try
  {
    if (mFileOutput)
    {
      mOutputFile << "Evolution optimization '" + mOptimizationName + "' ended\n" << std::endl;
      mOutputFile << "Evolution result: " << GetOutputFileString(-1, population.bestEntity.params, population.bestEntity.fitness) << std::endl;
      mOutputFile.close();
    }

    LOG_INFO("Evolution terminated: {}", GetTerminationReasonString(reason));
    LOG_INFO("Evolution result: {}", GetOutputFileString(-1, population.bestEntity.params, population.bestEntity.fitness));
    LOG_INFO("Evolution optimization ended");
  }
  catch (const std::exception &e)
  {
    LOG_ERROR("Could not uninitialize outputs: {}", e.what());
  }
}

Evolution::TerminationReason Evolution::CheckTerminationCriterions(const Population &population, int generation)
{
  if (population.bestEntity.fitness <= optimalFitness) // populationFitness goal reached
    return OptimalFitnessReached;

  if (generation >= maxGen) // maximum gen reached
    return MaximumGenerationsReached;

  if (population.functionEvaluations >= maxFunEvals) // maximum function evaluations exhausted
    return MaximumFunctionEvaluationsReached;

  if (population.relativeDifferenceGenerationsOverThreshold >
      mRelativeDifferenceGenerationsOverThresholdThreshold) // best entity fitness is almost the same as the average generation fitness - no
                                                            // improvement (relative)
    return NoImprovementReachedRel;

  if (population.absoluteDifference <
      mAbsoluteDifferenceThreshold) // best entity fitness is almost the same as the average generation fitness - no improvement (absolute)
    return NoImprovementReachedAbs;

  return NotTerminated;
}

std::string Evolution::GetOutputFileString(int gen, const std::vector<double> &bestEntity, double bestFitness)
{
  std::string value;
  if (gen >= 0)
    value += "Gen " + to_string(gen) + " (" + to_string(bestFitness) + "): [";
  else
    value += "(" + to_string(bestFitness) + "): [";

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
    functionEvaluations = 0;
    relativeDifferenceGenerationsOverThreshold = 0;
    InitializePopulation(NP, N, f, LB, UB);
    InitializeBestEntity();
    InitializeOffspring(nParents);
    return true;
  }
  catch (const std::exception &e)
  {
    LOG_ERROR("Could not initialize population: {}", e.what());
    return false;
  }
}

void Evolution::Population::UpdateDistinctParents(int eid) { offspring[eid].UpdateDistinctParents(eid, entities.size()); }

void Evolution::Population::UpdateCrossoverParameters(int eid, CrossoverStrategy crossoverStrategy, double CR)
{
  offspring[eid].UpdateCrossoverParameters(crossoverStrategy, CR);
}

void Evolution::Population::UpdateOffspring(
    int eid, MutationStrategy mutationStrategy, ObjectiveFunction f, double F, const std::vector<double> &LB, const std::vector<double> &UB)
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
        newoffspring.params[pid] = entities[newoffspring.parentIndices[0]].params[pid] +
                                   F * (entities[newoffspring.parentIndices[1]].params[pid] - entities[newoffspring.parentIndices[2]].params[pid]);
        break;
      case MutationStrategy::BEST1:
        newoffspring.params[pid] =
            bestEntity.params[pid] + F * (entities[newoffspring.parentIndices[0]].params[pid] - entities[newoffspring.parentIndices[1]].params[pid]);
        break;
      case MutationStrategy::RAND2:
        newoffspring.params[pid] = entities[newoffspring.parentIndices[0]].params[pid] +
                                   F * (entities[newoffspring.parentIndices[1]].params[pid] - entities[newoffspring.parentIndices[2]].params[pid]) +
                                   F * (entities[newoffspring.parentIndices[3]].params[pid] - entities[newoffspring.parentIndices[4]].params[pid]);
        break;
      case MutationStrategy::BEST2:
        newoffspring.params[pid] = bestEntity.params[pid] +
                                   F * (entities[newoffspring.parentIndices[0]].params[pid] - entities[newoffspring.parentIndices[1]].params[pid]) +
                                   F * (entities[newoffspring.parentIndices[2]].params[pid] - entities[newoffspring.parentIndices[3]].params[pid]);
        break;
      }
    }
    // check for boundaries, effectively clamp
    newoffspring.params[pid] = clampSmooth(newoffspring.params[pid], entities[eid].params[pid], LB[pid], UB[pid]);
  }

  try
  {
    newoffspring.fitness = f(newoffspring.params);
  }
  catch (const std::exception &e)
  {
    LOG_DEBUG("Could not evaluate new offspring with params {}: {}", newoffspring.params, e.what());
    newoffspring.fitness = Inf;
  }
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
  averageFitness = 0;
  previousFitness = bestEntity.fitness;
  for (int eid = 0; eid < entities.size(); ++eid)
  {
    averageFitness += entities[eid].fitness;
    if (entities[eid].fitness <= bestEntity.fitness)
      bestEntity = entities[eid];
  }
  averageFitness /= entities.size();
}

void Evolution::Population::UpdateTerminationCriterions(double relativeDifferenceThreshold)
{
  absoluteDifference = averageFitness - bestEntity.fitness;
  relativeDifference = bestEntity.fitness / averageFitness;

  if (relativeDifference > relativeDifferenceThreshold)
    relativeDifferenceGenerationsOverThreshold++;
  else
    relativeDifferenceGenerationsOverThreshold = 0;
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
        double avgDist =
            averageVectorDistance(entities[eid].params, entities[eidx2].params, RB); // calculate how distinct the entity is to another entity
        if (avgDist < minAvgDist)                                                    // check if entity is distinct
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

void Evolution::Population::InitializeOffspring(int nParents)
{
  LOG_INFO("Creating initial offspring...");
  offspring = zerovect(entities.size(), Offspring(entities[0].params.size(), nParents));
  for (int eid = 0; eid < entities.size(); eid++)
  {
    offspring[eid].params = entities[eid].params;
    offspring[eid].fitness = entities[eid].fitness;
  }
  LOG_SUCC("Initial offspring created");
}

void Evolution::Population::InitializeBestEntity()
{
  LOG_INFO("Searching for best entity in the initial population...");
  bestEntity = Entity(entities[0].params.size());
  UpdateBestEntity();
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
