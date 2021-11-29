#include "stdafx.h"
#include "Evolution.h"
#include "Plot/Plot.h"

Evolution::Evolution(int N, const std::string& optname) : OptimizationAlgorithm(N), mOptimizationName(optname), mNP(7 * N){};

OptimizationAlgorithm::OptimizationResult Evolution::Optimize(ObjectiveFunction obj, ValidationFunction valid)
try
{
  if (mConsoleOutput)
  {
    LOG_FUNCTION("Evolution optimization");
    LOG_INFO("Running evolution...");
  }

  InitializeOutputs(valid);
  CheckBounds();
  CheckObjectiveFunctionNormality(obj);
  CheckValidationFunctionNormality(valid);

  int gen = 0;
  Population population(mNP, N, obj, mLB, mUB, GetNumberOfParents(), mConsoleOutput);
  population.UpdateTerminationCriterions(mRelativeDifferenceThreshold);
  TerminationReason termReason = NotTerminated;
  UpdateOutputs(gen, population, valid);

  try
  {
    while (termReason == NotTerminated)
    {
      gen++;
#pragma omp parallel for
      for (int eid = 0; eid < mNP; ++eid)
      {
        population.UpdateDistinctParents(eid);
        population.UpdateCrossoverParameters(eid, mCrossStrat, mCR);
        population.UpdateOffspring(eid, mMutStrat, obj, mF, mLB, mUB);
      }
      population.PerformSelection();
      population.UpdateBestEntity();
      population.UpdateOffspringFunctionEvaluations();
      population.UpdateTerminationCriterions(mRelativeDifferenceThreshold);

      termReason = CheckTerminationCriterions(population, gen);
      UpdateOutputs(gen, population, valid);
    }
  }
  catch (const std::exception& e)
  {
    if (mConsoleOutput)
      LOG_ERROR("Unexpected error occured during generation {}: {}", gen, e.what());
    termReason = UnexpectedErrorOccured;
  }
  catch (...)
  {
    if (mConsoleOutput)
      LOG_ERROR("Unexpected error occured during generation {}", gen);
    termReason = UnexpectedErrorOccured;
  }

  UninitializeOutputs(population, termReason);
  OptimizationResult result;
  result.optimum = population.bestEntity.params;
  result.functionEvaluations = population.functionEvaluations;
  result.terminationReason = termReason;
  return result;
}
catch (const std::exception& e)
{
  if (mConsoleOutput)
    LOG_ERROR("Evolution optimization error: {}", e.what());
  return OptimizationResult();
}
catch (...)
{
  if (mConsoleOutput)
    LOG_ERROR("Unexpected evolution optimization error");
  return OptimizationResult();
}

void Evolution::SetFileOutputDir(const std::string& dir)
{
  mOutputFileDir = dir;
  mFileOutput = true;
}

void Evolution::MetaOptimize(ObjectiveFunction obj, int runsPerObj, int maxFunEvals, double optimalFitness)
{
  enum MetaParameters
  {
    NP,
    CR,
    F,
    MutationStrategy,
    CrossoverStrategy,
    MetaParametersCount
  };

  const auto metaobj = [&](const std::vector<double>& metaparams)
  {
    double retval = 0;

    for (int run = 0; run < runsPerObj; run++)
    {
      Evolution evo(N);
      evo.mLB = mLB;
      evo.mUB = mUB;
      evo.mNP = metaparams[NP];
      evo.mCR = metaparams[CR];
      evo.mF = metaparams[F];
      evo.SetConsoleOutput(false);
      evo.SetPlotOutput(false);
      evo.maxFunEvals = maxFunEvals;
      evo.optimalFitness = optimalFitness;

      const auto result = evo.Optimize(obj);
      retval += result.terminationReason == OptimalFitnessReached ? result.functionEvaluations : maxFunEvals;
    }

    return retval / runsPerObj;
  };

  Evolution evo(MetaParametersCount);
  evo.SetParameterNames({"NP", "CR", "F", "MutationStrategy", "CrossoverStrategy"});
  evo.mLB = {5, 0.1, 0.1, -1, -1};
  evo.mUB = {100, 1, 2, 1, 1};
  evo.mMutStrat = mMutStrat;
  evo.mCrossStrat = mCrossStrat;

  auto optimalMetaParams = evo.Optimize(metaobj).optimum;

  if (optimalMetaParams.size() != MetaParametersCount)
    return;

  mNP = optimalMetaParams[NP];
  mCR = optimalMetaParams[CR];
  mF = optimalMetaParams[F];
}

void Evolution::InitializeOutputs(ValidationFunction valid)
try
{
  if (mConsoleOutput)
    LOG_FUNCTION("Output initialization");

  if (mFileOutput)
  {
    mOutputFile.open(mOutputFileDir + mOptimizationName + ".txt", std::ios::out | std::ios::app);
    fmt::print(mOutputFile, "Evolution optimization '{}' started\n", mOptimizationName);
  }

  if (mPlotOutput)
  {
    Plot1D::Set("Evolution");
    Plot1D::Clear();
    Plot1D::SetXlabel("generation");
    Plot1D::SetYlabel("error");
    Plot1D::SetY2label("best-average relative difference");
    if (valid)
      Plot1D::SetYnames({"obj", "valid"});
    else
      Plot1D::SetYnames({"obj"});
    Plot1D::SetY2names({"reldiff"});
    Plot1D::SetPens({Plot::pens[0], Plot::pens[2], Plot::pens[1]});
  }
}
catch (const std::exception& e)
{
  throw std::runtime_error(fmt::format("Could not initialize outputs: {}", e.what()));
}

void Evolution::CheckObjectiveFunctionNormality(ObjectiveFunction obj)
try
{
  if (mConsoleOutput)
    LOG_FUNCTION("Objective function normality check");

  const auto arg = 0.5 * (mLB + mUB);
  const auto result1 = obj(arg);
  const auto result2 = obj(arg);

  if (result1 != result2)
    LOG_WARNING("Objective function is not consistent ({} != {})", result1, result2);
  else if (mConsoleOutput)
    LOG_TRACE("Objective function is consistent");

  if (!isfinite(result1))
    throw std::runtime_error(fmt::format("Objective function is not finite"));
  else if (mConsoleOutput)
    LOG_TRACE("Objective function is finite");

  if (result1 < 0)
    throw std::runtime_error(fmt::format("Objective function is not positive"));
  else if (mConsoleOutput)
    LOG_TRACE("Objective function is positive");
}
catch (const std::exception& e)
{
  throw std::runtime_error(fmt::format("Objective function is not normal: {}", e.what()));
}

void Evolution::CheckValidationFunctionNormality(ValidationFunction valid)
try
{
  if (!valid)
    return;

  if (mConsoleOutput)
    LOG_FUNCTION("Validation function normality check");

  const auto arg = 0.5 * (mLB + mUB);
  const auto result1 = valid(arg);
  const auto result2 = valid(arg);

  if (result1 != result2)
    throw std::runtime_error(fmt::format("Validation function is not consistent ({} != {})", result1, result2));
  else if (mConsoleOutput)
    LOG_TRACE("Validation function is consistent");

  if (!isfinite(result1))
    throw std::runtime_error(fmt::format("Validation function is not finite"));
  else if (mConsoleOutput)
    LOG_TRACE("Validation function is finite");

  if (result1 < 0)
    throw std::runtime_error(fmt::format("Validation function is not positive"));
  else if (mConsoleOutput)
    LOG_TRACE("Validation function is positive");
}
catch (const std::exception& e)
{
  throw std::runtime_error(fmt::format("Validation function is not normal: {}", e.what()));
}

void Evolution::CheckBounds()
{
  if (mConsoleOutput)
    LOG_FUNCTION("Objective function parameter bounds check");

  if (mLB.size() != mUB.size())
    throw std::runtime_error(fmt::format("Parameter bound sizes do not match"));
  if (mLB.size() != N)
    throw std::runtime_error(fmt::format("Invalid lower parameter bound size: {} != {}", mLB.size(), N));
  if (mUB.size() != N)
    throw std::runtime_error(fmt::format("Invalid upper parameter bound size: {} != {}", mUB.size(), N));
}

void Evolution::UpdateOutputs(int gen, const Population& population, ValidationFunction valid)
{
  if (population.bestEntity.fitness < population.previousFitness)
  {
    const auto message = GetOutputFileString(gen, population.bestEntity.params, population.bestEntity.fitness);

    if (mFileOutput)
      fmt::print(mOutputFile, "{}\n", message);

    if (mConsoleOutput)
      LOG_INFO("{}, reldif : {:.2f}, absdif : {:.2e}", message, population.relativeDifference, population.absoluteDifference);
  }

  if (mPlotOutput)
  {
    if (valid)
      Plot1D::Plot("Evolution", gen, {population.bestEntity.fitness, valid(population.bestEntity.params)}, {population.relativeDifference});
    else
      Plot1D::Plot("Evolution", gen, population.bestEntity.fitness, {population.relativeDifference});
  }
}

void Evolution::UninitializeOutputs(const Population& population, TerminationReason reason)
try
{
  if (mFileOutput)
  {
    fmt::print(mOutputFile, "Evolution optimization '{}' ended\n", mOptimizationName);
    fmt::print(mOutputFile, "Evolution result: {}\n", GetOutputFileString(-1, population.bestEntity.params, population.bestEntity.fitness));
    mOutputFile.close();
  }

  if (mConsoleOutput)
  {
    LOG_INFO("Evolution terminated: {}", GetTerminationReasonString(reason));
    LOG_SUCCESS("Evolution result: {}", GetOutputFileString(-1, population.bestEntity.params, population.bestEntity.fitness));
  }
}
catch (const std::exception& e)
{
  if (mConsoleOutput)
    LOG_ERROR("Could not uninitialize outputs: {}", e.what());
}

Evolution::TerminationReason Evolution::CheckTerminationCriterions(const Population& population, int generation)
{
  if (population.bestEntity.fitness <= optimalFitness) // populationFitness goal reached
    return OptimalFitnessReached;

  if (generation >= maxGen) // maximum gen reached
    return MaximumGenerationsReached;

  if (population.functionEvaluations >= maxFunEvals) // maximum function evaluations exhausted
    return MaximumFunctionEvaluationsReached;

  if (population.relativeDifferenceGenerationsOverThreshold > mRelativeDifferenceGenerationsOverThresholdThreshold) // best entity fitness is almost the same as the average generation fitness - no
                                                                                                                    // improvement (relative)
    return NoImprovementReachedRel;

  if (population.absoluteDifference < mAbsoluteDifferenceThreshold) // best entity fitness is almost the same as the average generation fitness - no improvement (absolute)
    return NoImprovementReachedAbs;

  return NotTerminated;
}

std::string Evolution::GetOutputFileString(int gen, const std::vector<double>& bestEntity, double bestFitness)
{
  std::string value;
  if (gen >= 0)
    value += fmt::format("Gen {} ({:.2e}) [", gen, bestFitness);
  else
    value += fmt::format("({:.2e}) [", bestFitness);

  for (int i = 0; i < bestEntity.size(); ++i)
  {
    if (mParameterNames.size() > i)
    {
      if (i < bestEntity.size() - 1)
        value += fmt::format("{}: {:.2f}, ", mParameterNames[i], bestEntity[i]);
      else
        value += fmt::format("{}: {:.2f}]", mParameterNames[i], bestEntity[i]);
    }
    else
    {
      if (i < bestEntity.size() - 1)
        value += fmt::format("param{}: {:.2f}, ", i, bestEntity[i]);
      else
        value += fmt::format("param{}: {:.2f}]", i, bestEntity[i]);
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

Evolution::Population::Population(int NP, int N, ObjectiveFunction obj, const std::vector<double>& LB, const std::vector<double>& UB, int nParents, bool consoleOutput)
try
{
  functionEvaluations = 0;
  relativeDifferenceGenerationsOverThreshold = 0;
  mConsoleOutput = consoleOutput;
  InitializePopulation(NP, N, obj, LB, UB);
  InitializeBestEntity();
  InitializeOffspring(nParents);
}
catch (const std::exception& e)
{
  throw std::runtime_error(fmt::format("Could not initialize population: {}", e.what()));
}

void Evolution::Population::UpdateDistinctParents(int eid)
{
  offspring[eid].UpdateDistinctParents(eid, entities.size());
}

void Evolution::Population::UpdateCrossoverParameters(int eid, CrossoverStrategy crossoverStrategy, double CR)
{
  offspring[eid].UpdateCrossoverParameters(crossoverStrategy, CR);
}

void Evolution::Population::UpdateOffspring(int eid, MutationStrategy mutationStrategy, ObjectiveFunction obj, double F, const std::vector<double>& LB, const std::vector<double>& UB)
{
  auto& newoffspring = offspring[eid];
  newoffspring.params = entities[eid].params;
  for (int pid = 0; pid < newoffspring.params.size(); pid++)
  {
    if (newoffspring.crossoverParameters[pid])
    {
      switch (mutationStrategy)
      {
      case MutationStrategy::RAND1:
        newoffspring.params[pid] =
            entities[newoffspring.parentIndices[0]].params[pid] + F * (entities[newoffspring.parentIndices[1]].params[pid] - entities[newoffspring.parentIndices[2]].params[pid]);
        break;
      case MutationStrategy::BEST1:
        newoffspring.params[pid] = bestEntity.params[pid] + F * (entities[newoffspring.parentIndices[0]].params[pid] - entities[newoffspring.parentIndices[1]].params[pid]);
        break;
      case MutationStrategy::RAND2:
        newoffspring.params[pid] = entities[newoffspring.parentIndices[0]].params[pid] +
                                   F * (entities[newoffspring.parentIndices[1]].params[pid] - entities[newoffspring.parentIndices[2]].params[pid]) +
                                   F * (entities[newoffspring.parentIndices[3]].params[pid] - entities[newoffspring.parentIndices[4]].params[pid]);
        break;
      case MutationStrategy::BEST2:
        newoffspring.params[pid] = bestEntity.params[pid] + F * (entities[newoffspring.parentIndices[0]].params[pid] - entities[newoffspring.parentIndices[1]].params[pid]) +
                                   F * (entities[newoffspring.parentIndices[2]].params[pid] - entities[newoffspring.parentIndices[3]].params[pid]);
        break;
      }
    }
    // check for boundaries, effectively clamp
    newoffspring.params[pid] = clampSmooth(newoffspring.params[pid], entities[eid].params[pid], LB[pid], UB[pid]);
  }

  try
  {
    newoffspring.fitness = obj(newoffspring.params);
  }
  catch (const std::exception& e)
  {
    if (mConsoleOutput)
      LOG_TRACE("Could not evaluate new offspring with params {}: {}", newoffspring.params, e.what());
    newoffspring.fitness = Constants::Inf;
  }
  catch (...)
  {
    if (mConsoleOutput)
      LOG_TRACE("Could not evaluate new offspring with params {}", newoffspring.params);
    newoffspring.fitness = Constants::Inf;
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

void Evolution::Population::UpdatePopulationFunctionEvaluations()
{
  functionEvaluations += entities.size();
}

void Evolution::Population::UpdateOffspringFunctionEvaluations()
{
  functionEvaluations += offspring.size();
}

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

void Evolution::Population::InitializePopulation(int NP, int N, ObjectiveFunction obj, const std::vector<double>& LB, const std::vector<double>& UB)
{
  if (mConsoleOutput)
    LOG_FUNCTION("Population initialization");
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

#pragma omp parallel for
  for (int eid = 0; eid < NP; eid++)
    entities[eid].fitness = obj(entities[eid].params);

  UpdatePopulationFunctionEvaluations();
}

void Evolution::Population::InitializeOffspring(int nParents)
{
  if (mConsoleOutput)
    LOG_FUNCTION("Offspring initialization");
  offspring = zerovect(entities.size(), Offspring(entities[0].params.size(), nParents));
  for (int eid = 0; eid < entities.size(); eid++)
  {
    offspring[eid].params = entities[eid].params;
    offspring[eid].fitness = entities[eid].fitness;
  }
}

void Evolution::Population::InitializeBestEntity()
{
  if (mConsoleOutput)
    LOG_FUNCTION("Best entity search");
  bestEntity = Entity(entities[0].params.size());
  UpdateBestEntity();
  if (mConsoleOutput)
    LOG_INFO("Initial population best entity: ({:.2e}) {}", bestEntity.fitness, bestEntity.params);
}

Evolution::Entity::Entity(int N)
{
  params.resize(N);
  fitness = Constants::Inf;
}

Evolution::Offspring::Offspring(int N, int nParents)
{
  params.resize(N);
  parentIndices.resize(nParents);
  crossoverParameters.resize(N);
  fitness = Constants::Inf;
}

void Evolution::Offspring::UpdateDistinctParents(int eid, int NP)
{
  for (auto& idx : parentIndices)
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
