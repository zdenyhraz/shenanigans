
#include "Evolution.hpp"
#include "Utils/Operators.hpp"
#include "Plot/Plot.hpp"

Evolution::Evolution(size_t N_, const std::string& optname) : OptimizationAlgorithm(N_, optname), mNP(7 * N)
{
  PROFILE_FUNCTION;
}

OptimizationAlgorithm::OptimizationResult Evolution::Optimize(const ObjectiveFunction& obj, const std::optional<ObjectiveFunction>& valid)
try
{
  PROFILE_FUNCTION;
  if (mConsoleOutput)
    LOG_INFO("Running evolution");

  CheckParameters();
  CheckBounds();
  CheckObjectiveFunctionNormality(obj);

  size_t gen = 0;
  Population population(mNP, N, obj, mLowerBounds, mUpperBounds, GetNumberOfParents(), mConsoleOutput, mSaveProgress);
  population.UpdateTerminationCriterions(mRelativeDifferenceThreshold);
  TerminationReason termReason = NotTerminated;
  UpdateOutputs(gen, population, valid);

  try
  {
    while (termReason == NotTerminated)
    {
      PROFILE_SCOPE(EvolutionGeneration);
      gen++;
#pragma omp parallel for
      for (int eid = 0; eid < mNP; ++eid)
      {
        population.UpdateDistinctParents(eid);
        population.UpdateCrossoverParameters(eid, mCrossStrat, mCR);
        population.UpdateOffspring(eid, mMutStrat, obj, mF, mLowerBounds, mUpperBounds);
      }
      population.functionEvaluations += mNP;
      population.PerformSelection();
      population.UpdateBestEntity();
      population.UpdateTerminationCriterions(mRelativeDifferenceThreshold);
      population.UpdateProgress();

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

  UninitializeOutputs(population, termReason, gen);
  OptimizationResult result;
  result.optimum = population.bestEntity.params;
  result.functionEvaluations = population.functionEvaluations;
  result.terminationReason = termReason;
  result.bestFitnessProgress = population.bestFitnessProgress;
  result.bestParametersProgress = population.bestParametersProgress;
  result.evaluatedParameters = population.evaluatedParameters;

  return result;
}
catch (const std::exception& e)
{
  if (mConsoleOutput)
    LOG_EXCEPTION(e);
  return OptimizationResult();
}
catch (...)
{
  if (mConsoleOutput)
    LOG_ERROR("Unexpected evolution optimization error");
  return OptimizationResult();
}

void Evolution::MetaOptimize(ObjectiveFunction obj, MetaObjectiveFunctionType metaObjType, size_t runsPerObj, size_t maxFunEvals, double optimalFitness)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  enum MetaParameter : uint8_t
  {
    MetaNP,
    MetaCR,
    MetaF,
    MetaMutationStrategy,
    MetaCrossoverStrategy,
    MetaParameterCount
  };

  const auto GetMetaParameterString = [](MetaParameter metaParameter)
  {
    switch (metaParameter)
    {
    case MetaNP:
      return "NP";
    case MetaCR:
      return "CR";
    case MetaF:
      return "F";
    case MetaMutationStrategy:
      return "Mutation strategy";
    case MetaCrossoverStrategy:
      return "Crossover strategy";
    default:
      throw std::runtime_error("Unknown meta parameter");
    }
  };

  const auto metaObj = [&](const std::vector<double>& metaparams)
  {
    double retval = 0;

    for (size_t run = 0; run < runsPerObj; run++)
    {
      Evolution evo(N);
      evo.mNP = metaparams[MetaNP];
      evo.mCR = metaparams[MetaCR];
      evo.mF = metaparams[MetaF];
      evo.mLowerBounds = mLowerBounds;
      evo.mUpperBounds = mUpperBounds;
      evo.mMaxFunEvals = maxFunEvals;
      evo.mOptimalFitness = optimalFitness;
      evo.mMutStrat = static_cast<Evolution::MutationStrategy>(static_cast<size_t>(metaparams[MetaMutationStrategy]));
      evo.mCrossStrat = static_cast<Evolution::CrossoverStrategy>(static_cast<size_t>(metaparams[MetaCrossoverStrategy]));
      evo.Mute();

      const auto result = evo.Optimize(obj);

      switch (metaObjType)
      {
      case ObjectiveFunctionValue:
        retval += obj(result.optimum);
        break;
      }
    }

    return retval / runsPerObj;
  };

  // set up the meta optimizer
  Evolution evo(MetaParameterCount, "metaopt");
  evo.SetParameterNames({"NP", "CR", "F", "Mutation strategy", "Crossover strategy"});
  evo.mNP = 10 * MetaParameterCount;
  evo.mMutStrat = BEST1;
  evo.mCrossStrat = BIN;
  evo.mMaxFunEvals = 5000;
  evo.mLowerBounds = {7, 0.1, 0.2, 0, 0};
  evo.mUpperBounds = {30. * N, 1, 1.5, -1e-6 + static_cast<double>(MutationStrategyCount), -1e-6 + static_cast<double>(CrossoverStrategyCount)};
  evo.SetConsoleOutput(true);
  evo.SetPlotOutput(true);
  evo.SetSaveProgress(true);

  // calculate metaopt parameters
  const auto metaOptResult = evo.Optimize(metaObj);
  const auto& optimalMetaParams = metaOptResult.optimum;
  if (optimalMetaParams.size() != MetaParameterCount)
    return;

  // save original settings and mute
  const auto consoleOutput = mConsoleOutput;
  const auto plotOutput = mPlotOutput;
  const auto saveProgress = mSaveProgress;
  Mute();
  SetSaveProgress(true);

  // statistics B4 metaopt
  LOG_INFO("Evolution parameters before metaoptimization: NP: {}, CR: {:.2f}, F: {:.2f}, M: {}, C: {}", mNP, mCR, mF, GetMutationStrategyString(mMutStrat),
      GetCrossoverStrategyString(mCrossStrat));
  std::vector<Evolution::OptimizationResult> resultsB4;
  for (size_t run = 0; run < runsPerObj; run++)
    resultsB4.push_back(Optimize(obj));

  // apply metaopt parameters
  mNP = optimalMetaParams[MetaNP];
  mCR = optimalMetaParams[MetaCR];
  mF = optimalMetaParams[MetaF];
  mMutStrat = static_cast<Evolution::MutationStrategy>(static_cast<size_t>(optimalMetaParams[MetaMutationStrategy]));
  mCrossStrat = static_cast<Evolution::CrossoverStrategy>(static_cast<size_t>(optimalMetaParams[MetaCrossoverStrategy]));

  // statistics A4 metaopt
  LOG_INFO("Evolution parameters after metaoptimization: NP: {}, CR: {:.2f}, F: {:.2f}, M: {}, C: {}", mNP, mCR, mF, GetMutationStrategyString(mMutStrat),
      GetCrossoverStrategyString(mCrossStrat));
  std::vector<Evolution::OptimizationResult> resultsA4;
  for (size_t run = 0; run < runsPerObj; run++)
    resultsA4.push_back(Optimize(obj));

  // plot unoptimized vs optimized
  std::vector<double> xs(resultsB4[0].bestFitnessProgress.size());
  std::iota(xs.begin(), xs.end(), 0);
  // Plot1D::Set(fmt::format("{} metaopt comparison", mName));
  // Plot1D::SetXlabel("generation");
  // Plot1D::SetYlabel("objective function value");
  // Plot1D::SetYnames({"obj B4", "obj A4"});
  // Plot1D::SetPens({Plot::pens[0], Plot::pens[2], Plot::pens[1]});
  // Plot1D::SetYLogarithmic(true);
  // Plot1D::Plot(xs, {resultsB4.front().bestFitnessProgress, resultsA4.front().bestFitnessProgress});

  // restore original settings
  SetConsoleOutput(consoleOutput);
  SetPlotOutput(plotOutput);
  SetSaveProgress(saveProgress);

  struct OptimizationPerformanceStatistics
  {
    double averageFitness = 0;
    double averageFunctionEvaluations = 0;
  };

  OptimizationPerformanceStatistics statsB4, statsA4;

  for (size_t run = 0; run < runsPerObj; run++)
  {
    statsB4.averageFitness += (1.0 / runsPerObj) * obj(resultsB4[run].optimum);
    statsA4.averageFitness += (1.0 / runsPerObj) * obj(resultsA4[run].optimum);

    statsB4.averageFunctionEvaluations += (1.0 / runsPerObj) * resultsB4[run].functionEvaluations;
    statsA4.averageFunctionEvaluations += (1.0 / runsPerObj) * resultsA4[run].functionEvaluations;
  }

  LOG_INFO("Before metaopt, reaching average fitness: {:.2e} took {:.0f} function evaluations ({:.0f}% of budget) on average ({} runs)", statsB4.averageFitness,
      statsB4.averageFunctionEvaluations, statsB4.averageFunctionEvaluations / maxFunEvals * 100, runsPerObj);
  LOG_INFO("After metaopt, reaching average fitness: {:.2e} took {:.0f} function evaluations ({:.0f}% of budget) on average ({} runs)", statsA4.averageFitness,
      statsA4.averageFunctionEvaluations, statsA4.averageFunctionEvaluations / maxFunEvals * 100, runsPerObj);

  if (statsA4.averageFitness < statsB4.averageFitness)
    LOG_INFO("Metaopt with {} function evaluations budget improved average resulting fitness from {:.2e} to {:.2e} ({} runs)", maxFunEvals, statsB4.averageFitness,
        statsA4.averageFitness, runsPerObj);
  else
    LOG_WARNING("Metaopt with {} function evaluations budget did not improve average resulting fitness ({} runs)", maxFunEvals, runsPerObj);
}

void Evolution::CheckObjectiveFunctionNormality(ObjectiveFunction obj) const
try
{
  PROFILE_FUNCTION;

  const auto arg = 0.5 * (mLowerBounds + mUpperBounds);
  const auto result1 = obj(arg);
  const auto result2 = obj(arg);

  if (result1 != result2)
  {
    if (mConsoleOutput)
      LOG_WARNING("Objective function is not consistent ({:.2e} != {:.2e})", result1, result2);
  }
  else if (mConsoleOutput)
    LOG_DEBUG("Objective function is consistent");

  if (!isfinite(result1))
    throw std::runtime_error(fmt::format("Objective function is not finite"));
  else if (mConsoleOutput)
    LOG_DEBUG("Objective function is finite");

  if (result1 < 0)
  {
    if (mConsoleOutput)
      LOG_WARNING("Objective function is not positive ({:.2e})", result1);
  }
  else if (mConsoleOutput)
    LOG_DEBUG("Objective function is positive");
}
catch (const std::exception& e)
{
  throw std::runtime_error(fmt::format("Objective function is not normal: {}", e.what()));
}

void Evolution::CheckBounds()
{
  PROFILE_FUNCTION;

  if (mLowerBounds.size() != mUpperBounds.size())
    throw std::runtime_error(fmt::format("Parameter bound sizes do not match"));
  if (mLowerBounds.size() != N)
    throw std::runtime_error(fmt::format("Invalid lower parameter bound size: {} != {}", mLowerBounds.size(), N));
  if (mUpperBounds.size() != N)
    throw std::runtime_error(fmt::format("Invalid upper parameter bound size: {} != {}", mUpperBounds.size(), N));
}

void Evolution::CheckParameters() const
{
  PROFILE_FUNCTION;

  if (mMutStrat == MutationStrategyCount)
    throw std::runtime_error("Invalid mutation strategy");

  if (mCrossStrat == CrossoverStrategyCount)
    throw std::runtime_error("Invalid crossover strategy");
}

void Evolution::UpdateOutputs(size_t generation, const Population& population, const std::optional<ObjectiveFunction>& valid)
{
  PROFILE_FUNCTION;

  if (mConsoleOutput)
    LOG_DEBUG(GetOutputString(generation, population));

  if (mPlotOutput)
  {
    static std::vector<double> gens;
    static std::vector<double> objvals;
    static std::vector<double> validvals;
    static std::vector<double> sims;

    if (generation == 0)
    {
      gens.clear();
      objvals.clear();
      validvals.clear();
      sims.clear();
    }

    gens.push_back(generation);
    objvals.push_back(population.bestEntity.fitness);
    if (valid)
      validvals.push_back(valid.value()(population.bestEntity.params));
    sims.push_back(population.relativeDifference * 100);

    Plot::Plot({.name = fmt::format("Evolution ({})", mName),
        .x = gens,
        .ys = {objvals, validvals},
        .y2s = {sims},
        .ylabels = {"obj", "valid"},
        .y2labels = {"%sim"},
        .xlabel = "generation",
        .ylabel = "objective function value",
        .y2label = "best-average relative similarity [%]",
        .log = false});

    Plot::Plot({.name = fmt::format("Evolution ({})", mName),
        .x = gens,
        .ys = {objvals, validvals},
        .ylabels = {"obj", "valid"},
        .xlabel = "generation",
        .ylabel = "objective function value",
        .log = false});
  }
}

void Evolution::UninitializeOutputs(const Population& population, TerminationReason reason, size_t generation)
try
{
  PROFILE_FUNCTION;

  if (mConsoleOutput)
  {
    LOG_INFO("Evolution terminated: {}", GetTerminationReasonString(reason));
    LOG_INFO("Evolution result: {}", GetOutputString(generation, population));
  }
}
catch (const std::exception& e)
{
  if (mConsoleOutput)
    LOG_EXCEPTION(e);
}

Evolution::TerminationReason Evolution::CheckTerminationCriterions(const Population& population, size_t generation) const
{
  if (population.bestEntity.fitness <= mOptimalFitness) // populationFitness goal reached
    return OptimalFitnessReached;

  if (generation >= maxGen) // maximum gen reached
    return MaximumGenerationsReached;

  if (population.functionEvaluations >= mMaxFunEvals) // maximum function evaluations exhausted
    return MaximumFunctionEvaluationsReached;

  if (generation > 10)
  {
    if (population.relativeDifferenceGenerationsOverThreshold >
        mRelativeDifferenceGenerationsOverThresholdThreshold) // best entity fitness is almost the same as the average generation fitness
      return NoImprovementReachedRel;

    if (population.absoluteDifference < mAbsoluteDifferenceThreshold) // best entity fitness is almost the same as the average generation fitness - no improvement (absolute)
      return NoImprovementReachedAbs;
  }

  return NotTerminated;
}

std::string Evolution::GetOutputString(size_t generation, const Population& population)
{
  PROFILE_FUNCTION;
  std::string value = fmt::format("Gen {} ({}) [", generation, population.bestEntity.fitness);
  for (size_t param = 0; param < population.bestEntity.params.size(); ++param)
    value += fmt::format("{}: {} ", mParameterNames[param], mParameterValueToNameFunctions[param](population.bestEntity.params[param]));
  value += fmt::format("] diff: {:.1f}% ({:.2e})", population.relativeDifference * 100, population.absoluteDifference);
  return value;
}

const char* Evolution::GetMutationStrategyString(MutationStrategy strategy)
{
  switch (strategy)
  {
  case RAND1:
    return "RAND1";
  case BEST1:
    return "BEST1";
  case RAND2:
    return "RAND2";
  case BEST2:
    return "BEST2";
  default:
    return "Unknown";
  }
}

const char* Evolution::GetCrossoverStrategyString(CrossoverStrategy strategy)
{
  switch (strategy)
  {
  case BIN:
    return "BIN";
  case EXP:
    return "EXP";
  default:
    return "Unknown";
  }
}

size_t Evolution::GetNumberOfParents() const
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
  default:
    throw std::runtime_error("Unknown mutation strategy");
  }
}

Evolution::Population::Population(
    size_t NP, size_t N, ObjectiveFunction obj, const std::vector<double>& LB, const std::vector<double>& UB, size_t nParents, bool consoleOutput, bool saveProgress)
try
{
  PROFILE_FUNCTION;
  functionEvaluations = 0;
  relativeDifferenceGenerationsOverThreshold = 0;
  mConsoleOutput = consoleOutput;
  mSaveProgress = saveProgress;

  if (mSaveProgress)
  {
    bestFitnessProgress.reserve(100);
    bestParametersProgress.reserve(100);
    evaluatedParameters.reserve(1000);
  }

  InitializePopulation(NP, N, obj, LB, UB);
  InitializeBestEntity();
  InitializeOffspring(nParents);
}
catch (const std::exception& e)
{
  throw std::runtime_error(fmt::format("Could not initialize population: {}", e.what()));
}

void Evolution::Population::UpdateDistinctParents(size_t eid)
{
  PROFILE_FUNCTION;
  offspring[eid].UpdateDistinctParents(eid, entities.size());
}

void Evolution::Population::UpdateCrossoverParameters(size_t eid, CrossoverStrategy crossoverStrategy, double CR)
{
  PROFILE_FUNCTION;
  offspring[eid].UpdateCrossoverParameters(crossoverStrategy, CR);
}

void Evolution::Population::UpdateOffspring(
    size_t eid, MutationStrategy mutationStrategy, ObjectiveFunction obj, double F, const std::vector<double>& LB, const std::vector<double>& UB)
{
  PROFILE_FUNCTION;
  auto& newoffspring = offspring[eid];
  newoffspring.params = entities[eid].params;
  for (size_t pid = 0; pid < newoffspring.params.size(); pid++)
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
      default:
        throw std::runtime_error("Unknown mutation strategy");
      }
    }
    // check for boundaries, effectively clamp
    newoffspring.params[pid] = ClampSmooth(newoffspring.params[pid], entities[eid].params[pid], LB[pid], UB[pid]);
  }

  try
  {
    newoffspring.fitness = obj(newoffspring.params);

    if (mSaveProgress)
#pragma omp critical
      evaluatedParameters.push_back(newoffspring.params);
  }
  catch (const std::exception& e)
  {
    if (mConsoleOutput)
      LOG_WARNING("Could not evaluate new offspring with params {}: {}", newoffspring.params, e.what());
    newoffspring.fitness = std::numeric_limits<double>::max();
  }
  catch (...)
  {
    if (mConsoleOutput)
      LOG_WARNING("Could not evaluate new offspring with params {}", newoffspring.params);
    newoffspring.fitness = std::numeric_limits<double>::max();
  }
}

void Evolution::Population::PerformSelection()
{
  PROFILE_FUNCTION;
  for (size_t eid = 0; eid < entities.size(); ++eid)
  {
    if (offspring[eid].fitness <= entities[eid].fitness)
    {
      entities[eid].params = offspring[eid].params;
      entities[eid].fitness = offspring[eid].fitness;
    }
  }
}

void Evolution::Population::UpdateBestEntity()
{
  PROFILE_FUNCTION;
  averageFitness = 0;
  previousFitness = bestEntity.fitness;
  for (size_t eid = 0; eid < entities.size(); ++eid)
  {
    averageFitness += entities[eid].fitness;
    if (entities[eid].fitness <= bestEntity.fitness)
      bestEntity = entities[eid];
  }
  averageFitness /= entities.size();
}

void Evolution::Population::UpdateTerminationCriterions(double relativeDifferenceThreshold)
{
  PROFILE_FUNCTION;
  absoluteDifference = averageFitness - bestEntity.fitness;
  relativeDifference = bestEntity.fitness / averageFitness;

  if (relativeDifference > relativeDifferenceThreshold)
    relativeDifferenceGenerationsOverThreshold++;
  else
    relativeDifferenceGenerationsOverThreshold = 0;
}

void Evolution::Population::UpdateProgress()
{
  PROFILE_FUNCTION;
  if (!mSaveProgress)
    return;

  bestFitnessProgress.push_back(bestEntity.fitness);
  bestParametersProgress.push_back(bestEntity.params);
}

void Evolution::Population::InitializePopulation(size_t NP, size_t N, ObjectiveFunction obj, const std::vector<double>& LB, const std::vector<double>& UB)
{
  PROFILE_FUNCTION;
  entities = Zerovect(NP, Entity(N));
  std::vector<double> RB = UB - LB;
  const double initialMinAvgDist = 0.5;
  const size_t distincEntityMaxTrials = 10;
  double minAvgDist = initialMinAvgDist;

  for (size_t eid = 0; eid < NP; eid++)
  {
    size_t distinctEntityTrials = 0;
    bool distinctEntity = false; // entities may not be too close together
    while (!distinctEntity)      // loop until they are distinct enough
    {
      distinctEntity = true; // assume entity is distinct

      for (size_t pid = 0; pid < N; pid++) // generate initial entity
        entities[eid].params[pid] = Random::Rand(LB[pid], UB[pid]);

      for (size_t eidx2 = 0; eidx2 < eid; eidx2++) // check distance to all other entities
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
  for (int eid = 0; eid < NP; ++eid)
  {
    entities[eid].fitness = obj(entities[eid].params);

    if (mSaveProgress)
#pragma omp critical
      evaluatedParameters.push_back(entities[eid].params);
  }
  functionEvaluations += NP;
}

void Evolution::Population::InitializeOffspring(size_t nParents)
{
  PROFILE_FUNCTION;
  offspring = Zerovect(entities.size(), Offspring(entities[0].params.size(), nParents));
  for (size_t eid = 0; eid < entities.size(); ++eid)
  {
    offspring[eid].params = entities[eid].params;
    offspring[eid].fitness = entities[eid].fitness;
  }
}

void Evolution::Population::InitializeBestEntity()
{
  PROFILE_FUNCTION;
  bestEntity = Entity(entities[0].params.size());
  UpdateBestEntity();
}

Evolution::Entity::Entity(size_t N)
{
  params.resize(N);
  fitness = std::numeric_limits<double>::max();
}

Evolution::Offspring::Offspring(size_t N, size_t nParents)
{
  params.resize(N);
  parentIndices.resize(nParents);
  crossoverParameters.resize(N);
  fitness = std::numeric_limits<double>::max();
}

void Evolution::Offspring::UpdateDistinctParents(size_t eid, size_t NP)
{
  PROFILE_FUNCTION;
  for (auto& idx : parentIndices)
  {
    size_t idxTst = Random::Rand<int>(0, 1e6) % NP;
    while (std::any_of(parentIndices.begin(), parentIndices.end(), [&](const auto pidx) { return idxTst == pidx; }) or idxTst == eid)
      idxTst = Random::Rand<int>(0, 1e6) % NP;
    idx = idxTst;
  }
}

void Evolution::Offspring::UpdateCrossoverParameters(CrossoverStrategy crossoverStrategy, double CR)
{
  PROFILE_FUNCTION;
  crossoverParameters = Zerovect(params.size(), false);

  switch (crossoverStrategy)
  {
  case CrossoverStrategy::BIN:
  {
    size_t definite = Random::Rand<int>(0, 1e6) % params.size(); // at least one param undergoes crossover
    for (size_t pid = 0; pid < params.size(); ++pid)
    {
      double random = Random::Rand();
      // flawfinder: ignore
      if (random < CR or pid == definite)
        crossoverParameters[pid] = true;
    }
    return;
  }
  case CrossoverStrategy::EXP:
  {
    size_t L = 1; // at least one param undergoes crossover
    while ((Random::Rand() < CR) and (L < params.size()))
      L++;

    size_t pid = Random::Rand<int>(0, 1e6) % params.size();
    for (size_t i = 0; i < L; i++)
    {
      crossoverParameters[pid] = true;
      pid++;
      pid %= params.size();
    }
    return;
  }
  default:
    throw std::runtime_error("Unknown crossover strategy");
  }
}

double Evolution::averageVectorDistance(const std::vector<double>& vec1, const std::vector<double>& vec2, const std::vector<double>& boundsRange)
{
  double result = 0;
  for (size_t i = 0; i < vec1.size(); i++)
    result += abs(vec1[i] - vec2[i]) / boundsRange[i]; // cv::normalize -> 0 to 1

  result /= vec1.size(); // coordinate average
  return result;
}
