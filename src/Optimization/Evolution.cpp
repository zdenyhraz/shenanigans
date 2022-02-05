
#include <fmt/format.h>
#include <fmt/ostream.h>

#include "Plot/Plot.h"
#include "Logger/Logger.h"
#include "Logger/LogFunction.h"
#include "Plot/Plot1D.h"
#include "Evolution.h"

Evolution::Evolution(usize N_, const std::string& optname) : OptimizationAlgorithm(N_, optname), mNP(7 * N_)
{
}

OptimizationAlgorithm::OptimizationResult Evolution::Optimize(ObjectiveFunction obj, ValidationFunction valid)
try
{
  PROFILE_EVENT();
  if (mConsoleOutput)
  {
    LOG_FUNCTION("Evolution optimization");
    LOG_INFO("Running evolution...");
  }

  CheckParameters();
  InitializeOutputs(valid);
  CheckBounds();
  CheckObjectiveFunctionNormality(obj);
  CheckValidationFunctionNormality(valid);

  usize gen = 0;
  Population population(mNP, N, obj, mLB, mUB, GetNumberOfParents(), mConsoleOutput, mSaveProgress);
  population.UpdateTerminationCriterions(mRelativeDifferenceThreshold);
  TerminationReason termReason = NotTerminated;
  UpdateOutputs(gen, population, valid);

  try
  {
    while (termReason == NotTerminated)
    {
      PROFILE_EVENT("Evolution generation");
      gen++;
#pragma omp parallel for
      for (usize eid = 0; eid < mNP; ++eid)
      {
        population.UpdateDistinctParents(eid);
        population.UpdateCrossoverParameters(eid, mCrossStrat, mCR);
        population.UpdateOffspring(eid, mMutStrat, obj, mF, mLB, mUB);
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

  if (mPlotObjectiveFunctionLandscape)
  {
    const usize xParamIndex = 0;
    const usize yParamIndex = 1;
    const f64 xmin = mLB[xParamIndex];
    const f64 xmax = mUB[xParamIndex];
    const f64 ymin = mLB[yParamIndex];
    const f64 ymax = mUB[yParamIndex];
    const auto baseParams = mLB + 0.5 * (mUB - mLB);
    PlotObjectiveFunctionLandscape(obj, baseParams, mPlotObjectiveFunctionLandscapeIterations, xParamIndex, yParamIndex, xmin, xmax, ymin, ymax, "x", "y", fmt::format("{} opt", mName), &result);
  }

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

void Evolution::MetaOptimize(ObjectiveFunction obj, MetaObjectiveFunctionType metaObjType, usize runsPerObj, usize maxFunEvals, f64 optimalFitness)
{
  PROFILE_EVENT();
  LOG_FUNCTION("Evolution metaoptimization");

  enum MetaParameter : u8
  {
    MetaNP,
    MetaCR,
    MetaF,
    MetaMutationStrategy,
    MetaCrossoverStrategy,
    MetaParameterCount
  };

  const auto GetMetaParameterString = [](MetaParameter metaParameter) {
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

  const auto metaObj = [&](const std::vector<f64>& metaparams) {
    f64 retval = 0;

    for (usize run = 0; run < runsPerObj; run++)
    {
      Evolution evo(N);
      evo.mNP = metaparams[MetaNP];
      evo.mCR = metaparams[MetaCR];
      evo.mF = metaparams[MetaF];
      evo.mLB = mLB;
      evo.mUB = mUB;
      evo.mMaxFunEvals = maxFunEvals;
      evo.mOptimalFitness = optimalFitness;
      evo.mMutStrat = static_cast<Evolution::MutationStrategy>(static_cast<usize>(metaparams[MetaMutationStrategy]));
      evo.mCrossStrat = static_cast<Evolution::CrossoverStrategy>(static_cast<usize>(metaparams[MetaCrossoverStrategy]));
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
  evo.mLB = {7, 0.1, 0.2, 0, 0};
  evo.mUB = {30. * N, 1, 1.5, -1e-6 + static_cast<f64>(MutationStrategyCount), -1e-6 + static_cast<f64>(CrossoverStrategyCount)};
  evo.SetConsoleOutput(true);
  evo.SetPlotOutput(true);
  evo.SetFileOutput(false);
  evo.SetPlotObjectiveFunctionLandscape(false);
  evo.SetSaveProgress(true);

  // calculate metaopt parameters
  const auto metaOptResult = evo.Optimize(metaObj);
  const auto& optimalMetaParams = metaOptResult.optimum;
  if (optimalMetaParams.size() != MetaParameterCount)
    return;

  if (mPlotObjectiveFunctionLandscape)
  {
    // plot metaopt surface
    const usize xParamIndex = MetaNP;
    const usize yParamIndex = MetaMutationStrategy;
    const f64 xmin = evo.mLB[xParamIndex];
    const f64 xmax = evo.mUB[xParamIndex];
    const f64 ymin = evo.mLB[yParamIndex];
    const f64 ymax = evo.mUB[yParamIndex];
    const auto baseParams = std::vector<f64>{20., mCR, mF, (f64)RAND1, (f64)BIN};
    PlotObjectiveFunctionLandscape(metaObj, baseParams, mPlotObjectiveFunctionLandscapeIterations, xParamIndex, yParamIndex, xmin, xmax, ymin, ymax,
        GetMetaParameterString(static_cast<MetaParameter>(xParamIndex)), GetMetaParameterString(static_cast<MetaParameter>(yParamIndex)), fmt::format("{} metaopt", mName), &metaOptResult);
  }

  // save original settings and mute
  const auto consoleOutput = mConsoleOutput;
  const auto plotOutput = mPlotOutput;
  const auto plotObjFunLandscape = mPlotObjectiveFunctionLandscape;
  const auto saveProgress = mSaveProgress;
  Mute();
  SetSaveProgress(true);

  // statistics B4 metaopt
  LOG_INFO("Evolution parameters before metaoptimization: NP: {}, CR: {:.2f}, F: {:.2f}, M: {}, C: {}", mNP, mCR, mF, GetMutationStrategyString(mMutStrat), GetCrossoverStrategyString(mCrossStrat));
  std::vector<Evolution::OptimizationResult> resultsB4;
  for (usize run = 0; run < runsPerObj; run++)
    resultsB4.push_back(Optimize(obj));

  // apply metaopt parameters
  mNP = optimalMetaParams[MetaNP];
  mCR = optimalMetaParams[MetaCR];
  mF = optimalMetaParams[MetaF];
  mMutStrat = static_cast<Evolution::MutationStrategy>(static_cast<usize>(optimalMetaParams[MetaMutationStrategy]));
  mCrossStrat = static_cast<Evolution::CrossoverStrategy>(static_cast<usize>(optimalMetaParams[MetaCrossoverStrategy]));

  // statistics A4 metaopt
  LOG_INFO("Evolution parameters after metaoptimization: NP: {}, CR: {:.2f}, F: {:.2f}, M: {}, C: {}", mNP, mCR, mF, GetMutationStrategyString(mMutStrat), GetCrossoverStrategyString(mCrossStrat));
  std::vector<Evolution::OptimizationResult> resultsA4;
  for (usize run = 0; run < runsPerObj; run++)
    resultsA4.push_back(Optimize(obj));

  // plot unoptimized vs optimized
  std::vector<f64> xs(resultsB4[0].bestFitnessProgress.size());
  std::iota(xs.begin(), xs.end(), 0);
  Plot1D::Set(fmt::format("{} metaopt comparison", mName));
  Plot1D::SetXlabel("generation");
  Plot1D::SetYlabel("objective function value");
  Plot1D::SetYnames({"obj B4", "obj A4"});
  Plot1D::SetPens({Plot::pens[0], Plot::pens[2], Plot::pens[1]});
  Plot1D::SetYLogarithmic(true);
  Plot1D::Plot(xs, {resultsB4.front().bestFitnessProgress, resultsA4.front().bestFitnessProgress});

  // restore original settings
  SetConsoleOutput(consoleOutput);
  SetPlotOutput(plotOutput);
  SetPlotObjectiveFunctionLandscape(plotObjFunLandscape);
  SetSaveProgress(saveProgress);

  struct OptimizationPerformanceStatistics
  {
    f64 averageFitness = 0;
    f64 averageFunctionEvaluations = 0;
  };

  OptimizationPerformanceStatistics statsB4, statsA4;

  for (usize run = 0; run < runsPerObj; run++)
  {
    statsB4.averageFitness += (1.0 / runsPerObj) * obj(resultsB4[run].optimum);
    statsA4.averageFitness += (1.0 / runsPerObj) * obj(resultsA4[run].optimum);

    statsB4.averageFunctionEvaluations += (1.0 / runsPerObj) * resultsB4[run].functionEvaluations;
    statsA4.averageFunctionEvaluations += (1.0 / runsPerObj) * resultsA4[run].functionEvaluations;
  }

  LOG_INFO("Before metaopt, reaching average fitness: {:.2e} took {:.0f} function evaluations ({:.0f}% of budget) on average ({} runs)", statsB4.averageFitness, statsB4.averageFunctionEvaluations,
      statsB4.averageFunctionEvaluations / maxFunEvals * 100, runsPerObj);
  LOG_INFO("After metaopt, reaching average fitness: {:.2e} took {:.0f} function evaluations ({:.0f}% of budget) on average ({} runs)", statsA4.averageFitness, statsA4.averageFunctionEvaluations,
      statsA4.averageFunctionEvaluations / maxFunEvals * 100, runsPerObj);

  if (statsA4.averageFitness < statsB4.averageFitness)
    LOG_SUCCESS(
        "Metaopt with {} function evaluations budget improved average resulting fitness from {:.2e} to {:.2e} ({} runs)", maxFunEvals, statsB4.averageFitness, statsA4.averageFitness, runsPerObj);
  else
    LOG_WARNING("Metaopt with {} function evaluations budget did not improve average resulting fitness ({} runs)", maxFunEvals, runsPerObj);
}

void Evolution::InitializeOutputs(ValidationFunction valid)
try
{
  PROFILE_EVENT();
  if (mConsoleOutput)
    LOG_FUNCTION("Output initialization");

  if (mFileOutput)
  {
    mOutputFile.open(mOutputFileDir + mName + ".txt", std::ios::out | std::ios::app);
    fmt::print(mOutputFile, "Evolution optimization '{}' started\n", mName);
  }

  if (mPlotOutput)
  {
    Plot1D::Set(fmt::format("Evolution ({})", mName));
    Plot1D::Clear();
    Plot1D::SetXlabel("generation");
    Plot1D::SetYlabel("objective function value");
    Plot1D::SetY2label("best-average relative difference [%]");
    if (valid)
      Plot1D::SetYnames({"obj", "valid"});
    else
      Plot1D::SetYnames({"obj"});
    Plot1D::SetY2names({"%diff"});
    Plot1D::SetPens({Plot::pens[0], Plot::pens[2], Plot::pens[1]});
    Plot1D::SetYLogarithmic(true);
  }
}
catch (const std::exception& e)
{
  throw std::runtime_error(fmt::format("Could not initialize outputs: {}", e.what()));
}

void Evolution::CheckObjectiveFunctionNormality(ObjectiveFunction obj)
try
{
  PROFILE_EVENT();
  if (mConsoleOutput)
    LOG_FUNCTION("Objective function normality check");

  const auto arg = 0.5 * (mLB + mUB);
  const auto result1 = obj(arg);
  const auto result2 = obj(arg);

  if (result1 != result2)
  {
    if (mConsoleOutput)
      LOG_WARNING("Objective function is not consistent ({:.2e} != {:.2e})", result1, result2);
  }
  else if (mConsoleOutput)
    LOG_TRACE("Objective function is consistent");

  if (!isfinite(result1))
    throw std::runtime_error(fmt::format("Objective function is not finite"));
  else if (mConsoleOutput)
    LOG_TRACE("Objective function is finite");

  if (result1 < 0)
  {
    if (mConsoleOutput)
      LOG_WARNING("Objective function is not positive ({:.2e})", result1);
  }
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
  PROFILE_EVENT();
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
  PROFILE_EVENT();
  if (mConsoleOutput)
    LOG_FUNCTION("Objective function parameter bounds check");

  if (mLB.size() != mUB.size())
    throw std::runtime_error(fmt::format("Parameter bound sizes do not match"));
  if (mLB.size() != N)
    throw std::runtime_error(fmt::format("Invalid lower parameter bound size: {} != {}", mLB.size(), N));
  if (mUB.size() != N)
    throw std::runtime_error(fmt::format("Invalid upper parameter bound size: {} != {}", mUB.size(), N));
}

void Evolution::CheckParameters()
{
  PROFILE_EVENT();
  if (mMutStrat == MutationStrategyCount)
    throw std::runtime_error("Invalid mutation strategy");

  if (mCrossStrat == CrossoverStrategyCount)
    throw std::runtime_error("Invalid crossover strategy");
}

void Evolution::UpdateOutputs(usize gen, const Population& population, ValidationFunction valid)
{
  PROFILE_EVENT();
  if (population.bestEntity.fitness < population.previousFitness)
  {
    if (mFileOutput)
      fmt::print(mOutputFile, "{}\n", GetOutputString(gen, population));

    if (mConsoleOutput)
      LOG_DEBUG(GetOutputString(gen, population));
  }

  if (mPlotOutput)
  {
    if (valid)
      Plot1D::Plot(gen, {population.bestEntity.fitness, valid(population.bestEntity.params)}, {population.relativeDifference * 100});
    else
      Plot1D::Plot(gen, population.bestEntity.fitness, {population.relativeDifference * 100});
  }
}

void Evolution::UninitializeOutputs(const Population& population, TerminationReason reason, usize generation)
try
{
  PROFILE_EVENT();
  if (mFileOutput)
  {
    fmt::print(mOutputFile, "Evolution optimization '{}' ended\n", mName);
    fmt::print(mOutputFile, "Evolution result: {}\n", GetOutputString(generation, population));
    mOutputFile.close();
  }

  if (mConsoleOutput)
  {
    LOG_INFO("Evolution terminated: {}", GetTerminationReasonString(reason));
    LOG_SUCCESS("Evolution result: {}", GetOutputString(generation, population));
  }
}
catch (const std::exception& e)
{
  if (mConsoleOutput)
    LOG_ERROR("Could not uninitialize outputs: {}", e.what());
}

Evolution::TerminationReason Evolution::CheckTerminationCriterions(const Population& population, usize generation)
{
  if (population.bestEntity.fitness <= mOptimalFitness) // populationFitness goal reached
    return OptimalFitnessReached;

  if (generation >= maxGen) // maximum gen reached
    return MaximumGenerationsReached;

  if (population.functionEvaluations >= mMaxFunEvals) // maximum function evaluations exhausted
    return MaximumFunctionEvaluationsReached;

  if (population.relativeDifferenceGenerationsOverThreshold > mRelativeDifferenceGenerationsOverThresholdThreshold) // best entity fitness is almost the same as the average generation fitness
    return NoImprovementReachedRel;

  if (population.absoluteDifference < mAbsoluteDifferenceThreshold) // best entity fitness is almost the same as the average generation fitness - no improvement (absolute)
    return NoImprovementReachedAbs;

  return NotTerminated;
}

std::string Evolution::GetOutputString(usize gen, const Population& population)
{
  PROFILE_EVENT();
  std::string value = fmt::format("Gen {} ({:.2e}) [", gen, population.bestEntity.fitness);
  for (usize param = 0; param < population.bestEntity.params.size(); ++param)
    value += fmt::format("{}: {} ", mParameterNames[param], mParameterValueToNameFunctions[param](population.bestEntity.params[param]));
  value += fmt::format("] diff: {:.2f}% ({:.2e})", population.relativeDifference * 100, population.absoluteDifference);
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

usize Evolution::GetNumberOfParents()
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

Evolution::Population::Population(usize NP, usize N, ObjectiveFunction obj, const std::vector<f64>& LB, const std::vector<f64>& UB, usize nParents, bool consoleOutput, bool saveProgress)
try
{
  PROFILE_EVENT();
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

void Evolution::Population::UpdateDistinctParents(usize eid)
{
  PROFILE_EVENT();
  offspring[eid].UpdateDistinctParents(eid, entities.size());
}

void Evolution::Population::UpdateCrossoverParameters(usize eid, CrossoverStrategy crossoverStrategy, f64 CR)
{
  PROFILE_EVENT();
  offspring[eid].UpdateCrossoverParameters(crossoverStrategy, CR);
}

void Evolution::Population::UpdateOffspring(usize eid, MutationStrategy mutationStrategy, ObjectiveFunction obj, f64 F, const std::vector<f64>& LB, const std::vector<f64>& UB)
{
  PROFILE_EVENT();
  auto& newoffspring = offspring[eid];
  newoffspring.params = entities[eid].params;
  for (usize pid = 0; pid < newoffspring.params.size(); pid++)
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
    newoffspring.params[pid] = clampSmooth(newoffspring.params[pid], entities[eid].params[pid], LB[pid], UB[pid]);
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
  PROFILE_EVENT();
  for (usize eid = 0; eid < entities.size(); ++eid)
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
  PROFILE_EVENT();
  averageFitness = 0;
  previousFitness = bestEntity.fitness;
  for (usize eid = 0; eid < entities.size(); ++eid)
  {
    averageFitness += entities[eid].fitness;
    if (entities[eid].fitness <= bestEntity.fitness)
      bestEntity = entities[eid];
  }
  averageFitness /= entities.size();
}

void Evolution::Population::UpdateTerminationCriterions(f64 relativeDifferenceThreshold)
{
  PROFILE_EVENT();
  absoluteDifference = averageFitness - bestEntity.fitness;
  relativeDifference = bestEntity.fitness / averageFitness;

  if (relativeDifference > relativeDifferenceThreshold)
    relativeDifferenceGenerationsOverThreshold++;
  else
    relativeDifferenceGenerationsOverThreshold = 0;
}

void Evolution::Population::UpdateProgress()
{
  PROFILE_EVENT();
  if (!mSaveProgress)
    return;

  bestFitnessProgress.push_back(bestEntity.fitness);
  bestParametersProgress.push_back(bestEntity.params);
}

void Evolution::Population::InitializePopulation(usize NP, usize N, ObjectiveFunction obj, const std::vector<f64>& LB, const std::vector<f64>& UB)
{
  PROFILE_EVENT();
  if (mConsoleOutput)
    LOG_FUNCTION("Population initialization");
  entities = zerovect(NP, Entity(N));
  std::vector<f64> RB = UB - LB;
  const f64 initialMinAvgDist = 0.5;
  const usize distincEntityMaxTrials = 10;
  f64 minAvgDist = initialMinAvgDist;

  for (usize eid = 0; eid < NP; eid++)
  {
    usize distinctEntityTrials = 0;
    bool distinctEntity = false; // entities may not be too close together
    while (!distinctEntity)      // loop until they are distinct enough
    {
      distinctEntity = true; // assume entity is distinct

      for (usize pid = 0; pid < N; pid++) // generate initial entity
        entities[eid].params[pid] = randr(LB[pid], UB[pid]);

      if (distincEntityMaxTrials < 1)
        break;

      for (usize eidx2 = 0; eidx2 < eid; eidx2++) // check distance to all other entities
      {
        f64 avgDist = averageVectorDistance(entities[eid].params, entities[eidx2].params, RB); // calculate how distinct the entity is to another entity
        if (avgDist < minAvgDist)                                                              // check if entity is distinct
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
  for (usize eid = 0; eid < NP; eid++)
  {
    entities[eid].fitness = obj(entities[eid].params);

    if (mSaveProgress)
#pragma omp critical
      evaluatedParameters.push_back(entities[eid].params);
  }
  functionEvaluations += NP;
}

void Evolution::Population::InitializeOffspring(usize nParents)
{
  PROFILE_EVENT();
  if (mConsoleOutput)
    LOG_FUNCTION("Offspring initialization");
  offspring = zerovect(entities.size(), Offspring(entities[0].params.size(), nParents));
  for (usize eid = 0; eid < entities.size(); eid++)
  {
    offspring[eid].params = entities[eid].params;
    offspring[eid].fitness = entities[eid].fitness;
  }
}

void Evolution::Population::InitializeBestEntity()
{
  PROFILE_EVENT();
  if (mConsoleOutput)
    LOG_FUNCTION("Best entity search");
  bestEntity = Entity(entities[0].params.size());
  UpdateBestEntity();
}

Evolution::Entity::Entity(usize N)
{
  params.resize(N);
  fitness = Constants::Inf;
}

Evolution::Offspring::Offspring(usize N, usize nParents)
{
  params.resize(N);
  parentIndices.resize(nParents);
  crossoverParameters.resize(N);
  fitness = Constants::Inf;
}

void Evolution::Offspring::UpdateDistinctParents(usize eid, usize NP)
{
  PROFILE_EVENT();
  for (auto& idx : parentIndices)
  {
    usize idxTst = rand() % NP;
    while (!isDistinct(idxTst, parentIndices, eid))
      idxTst = rand() % NP;
    idx = idxTst;
  }
}

void Evolution::Offspring::UpdateCrossoverParameters(CrossoverStrategy crossoverStrategy, f64 CR)
{
  PROFILE_EVENT();
  crossoverParameters = zerovect(params.size(), false);

  switch (crossoverStrategy)
  {
  case CrossoverStrategy::BIN:
  {
    usize definite = rand() % params.size(); // at least one param undergoes crossover
    for (usize pid = 0; pid < params.size(); pid++)
    {
      f64 random = rand01();
      if (random < CR or pid == definite)
        crossoverParameters[pid] = true;
    }
    return;
  }
  case CrossoverStrategy::EXP:
  {
    usize L = 0;
    do
      L++; // at least one param undergoes crossover
    while ((rand01() < CR) and (L < params.size()));

    usize pid = rand() % params.size();
    for (usize i = 0; i < L; i++)
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

f64 Evolution::averageVectorDistance(std::vector<f64>& vec1, std::vector<f64>& vec2, std::vector<f64>& boundsRange)
{
  PROFILE_EVENT();
  f64 result = 0;
  for (usize i = 0; i < vec1.size(); i++)
    result += abs(vec1[i] - vec2[i]) / boundsRange[i]; // normalize -> 0 to 1

  result /= vec1.size(); // coordinate average
  return result;
}

bool Evolution::isDistinct(usize inpindex, std::vector<usize>& indices, usize currindex)
{
  PROFILE_EVENT();
  bool isdist = true;
  for (auto& idx : indices)
  {
    if (inpindex == idx or inpindex == currindex)
      isdist = false;
  }
  return isdist;
}