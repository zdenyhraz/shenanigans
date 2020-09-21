#include "stdafx.h"
#include "Evolution.h"
#include "Plot/Plot1D.h"

Evolution::Evolution(int N) : OptimizationAlgorithm(N), mNP(mINPm * N){};

std::vector<double> Evolution::optimize(const std::function<double(const std::vector<double> &)> &f)
{
  LOG_STARTEND("Evolution optimization started", "Evolution optimization ended");
  Plot1D::Reset("evolution");
  std::ofstream file("E:\\Zdeny_PhD_Shenanigans\\articles\\diffrot\\temp\\opt.txt");
  file << "Evolution started." << std::endl;
  funEvals = 0;
  success = false;

  LOG_INFO("Checking objective function normality...");
  if (!isfinite(f(0.5 * (lowerBounds + upperBounds))))
  {
    LOG_ERROR("Objective function is not normal!");
    return {};
  }
  else
    LOG_SUCC("Objective function is normal");

  // establish population matrix, fitness vector and other internals
  vector<double> boundsRange = upperBounds - lowerBounds;
  vector<vector<double>> visitedPointsMainThisRun;
  vector<vector<double>> visitedPointsThisRun;
  double averageImprovement = 0;
  vector<vector<double>> population(mNP, zerovect(N, 0.));
  vector<queue<double>> histories(mNP);
  bool historyConstant = false;
  vector<double> fitness = zerovect(mNP, 0.);
  vector<double> bestEntity = zerovect(N, 0.);
  double bestFitness = Constants::Inf;
  double fitness_prev = Constants::Inf;
  double fitness_curr = Constants::Inf;

  // initialize random starting population matrix within bounds
  LOG_INFO("Creating initial population within bounds ... ");
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

  // calculate initial fitness vector
  LOG_INFO("Evaluating initial population...");
#pragma omp parallel for
  for (int indexEntity = 0; indexEntity < mNP; indexEntity++)
  {
    fitness[indexEntity] = f(population[indexEntity]);
    if (logPoints)
    {
#pragma omp critical
      visitedPointsThisRun.push_back(population[indexEntity]);
    }
  }
  funEvals += mNP;
  LOG_SUCC("Initial population evaluated");

  // determine the best entity in the initial population
  for (int indexEntity = 0; indexEntity < mNP; indexEntity++)
  {
    if (fitness[indexEntity] <= bestFitness)
    {
      bestEntity = population[indexEntity];
      bestFitness = fitness[indexEntity];
    }
  }
  LOG_INFO("Initial population best entity: {} ({:.3f})", bestEntity, bestFitness);
  file << "Init best entity: " + to_string(bestEntity) + " (" + to_string(bestFitness) + ")" << std::endl;
  Plot1D::plot(0, bestFitness, log(bestFitness), "evolution", "generation", "fitness", "log fitness");

  // run main evolution cycle
  LOG_INFO("Running evolution...");
  for (int generation = 1; generation < 1e8; generation++)
  {
#pragma omp parallel for
    for (int indexEntity = 0; indexEntity < mNP; indexEntity++)
    {
      // create new potential entity
      vector<double> newEntity = population[indexEntity];
      double newFitness = 0.;

      // select distinct parents different from the current entity
      int numberOfParents;
      // calculate the number of parents
      switch (mMutStrat)
      {
      case MutationStrategy::RAND1:
        numberOfParents = 3;
        break;
      case MutationStrategy::BEST1:
        numberOfParents = 2;
        break;
      case MutationStrategy::RAND2:
        numberOfParents = 5;
        break;
      case MutationStrategy::BEST2:
        numberOfParents = 4;
        break;
      }

      vector<int> parentIndices(numberOfParents, 0);
      for (auto &idx : parentIndices)
      {
        int idxTst;
        do
          idxTst = rand() % mNP;
        while (!isDistinct(idxTst, parentIndices, indexEntity));
        idx = idxTst;
      }

      // decide which parameters undergo crossover
      vector<bool> paramIsCrossed(N, false);
      switch (mCrossStrat)
      {
      case CrossoverStrategy::BIN:
      {
        int definite = rand() % N; // at least one param undergoes crossover
        for (int indexParam = 0; indexParam < N; indexParam++)
        {
          double random = rand01();
          if (random < mCR || indexParam == definite)
            paramIsCrossed[indexParam] = true;
        }
        break;
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
          paramIsCrossed[indexParam] = true;
          indexParam++;
          indexParam %= N;
        }
        break;
      }
      }

      // perform the crossover
      for (int indexParam = 0; indexParam < N; indexParam++)
      {
        if (paramIsCrossed[indexParam])
        {
          switch (mMutStrat)
          {
          case MutationStrategy::RAND1:
            newEntity[indexParam] = population[parentIndices[0]][indexParam] + mF * (population[parentIndices[1]][indexParam] - population[parentIndices[2]][indexParam]);
            break;
          case MutationStrategy::BEST1:
            newEntity[indexParam] = bestEntity[indexParam] + mF * (population[parentIndices[0]][indexParam] - population[parentIndices[1]][indexParam]);
            break;
          case MutationStrategy::RAND2:
            newEntity[indexParam] = population[parentIndices[0]][indexParam] + mF * (population[parentIndices[1]][indexParam] - population[parentIndices[2]][indexParam]) + mF * (population[parentIndices[3]][indexParam] - population[parentIndices[4]][indexParam]);
            break;
          case MutationStrategy::BEST2:
            newEntity[indexParam] = bestEntity[indexParam] + mF * (population[parentIndices[0]][indexParam] - population[parentIndices[1]][indexParam]) + mF * (population[parentIndices[2]][indexParam] - population[parentIndices[3]][indexParam]);
            break;
          }
        }
        // check for boundaries, effectively clamp
        newEntity[indexParam] = clampSmooth(newEntity[indexParam], population[indexEntity][indexParam], lowerBounds[indexParam], upperBounds[indexParam]);
      }

      // evaluate fitness of new entity
      newFitness = f(newEntity);

      if (logPoints)
      {
#pragma omp critical
        visitedPointsThisRun.push_back(newEntity);
      }

      // select the more fit entity
      if (newFitness <= fitness[indexEntity])
      {
        population[indexEntity] = newEntity;
        fitness[indexEntity] = newFitness;
      }

    } // entity cycle end

    funEvals += mNP;

    // determine the best entity
    for (int indexEntity = 0; indexEntity < mNP; indexEntity++)
    {
      if (fitness[indexEntity] <= bestFitness)
      {
        bestEntity = population[indexEntity];
        bestFitness = fitness[indexEntity];
        fitness_prev = fitness_curr;
        fitness_curr = bestFitness;
        LOG_SUCC("Gen {} best entity: {} ({:.5f}), CBI = {:.1f}%, AHI = {:.1f}%", generation, bestEntity, bestFitness, (fitness_prev - fitness_curr) / fitness_prev * 100, averageImprovement * 100);
        file << "Gen " + to_string(generation) + " best entity: " + to_string(bestEntity) + " (" + to_string(bestFitness) + ")" << std::endl;
        Plot1D::plot(generation, bestFitness, log(bestFitness), "evolution", "generation", "fitness", "log fitness");
      }
    }

    // fill history ques for all entities - termination criterion
    historyConstant = true; // assume history is constant
    averageImprovement = 0;
    for (int indexEntity = 0; indexEntity < mNP; indexEntity++)
    {
      if (histories[indexEntity].size() == mHistorySize)
      {
        histories[indexEntity].pop();                      // remove first element - keep que size constant
        histories[indexEntity].push(fitness[indexEntity]); // insert at the end
        if (mStopCrit == StoppingCriterion::ALLIMP)        // fitness improved less than x% for all entities
          if (abs(histories[indexEntity].front() - histories[indexEntity].back()) / abs(histories[indexEntity].front()) > mHistoryImprovTresholdPercent / 100)
            historyConstant = false;
      }
      else
      {
        histories[indexEntity].push(fitness[indexEntity]); // insert at the end
        historyConstant = false;                           // too early to stop, go on
      }
      if (histories[indexEntity].size() > 2)
        averageImprovement += abs(histories[indexEntity].front()) == 0 ? 0 : abs(histories[indexEntity].front() - histories[indexEntity].back()) / abs(histories[indexEntity].front());
    }
    averageImprovement /= mNP;
    if (mStopCrit == StoppingCriterion::AVGIMP) // average fitness improved less than x%
      if (100 * averageImprovement > mHistoryImprovTresholdPercent)
        historyConstant = false;

    // termination criterions
    if (bestFitness < optimalFitness) // fitness goal reached
    {
      LOG_SUCC("OptimalFitness value reached, terminating - generation " + to_string(generation) + ".\n");
      terminationReason = "optimalFitness value reached, final fitness: " + to_string(bestFitness);
      success = true;
      break;
    }
    if (generation == maxGen) // maximum generation reached
    {
      LOG_SUCC("MaxGen value reached, terminating - generation " + to_string(generation) + ".\n");
      terminationReason = "maxGen value reached, final fitness: " + to_string(bestFitness);
      success = false;
      break;
    }
    if (funEvals >= maxFunEvals) // maximum function evaluations exhausted
    {
      LOG_SUCC("MaxFunEvals value reached, terminating - generation " + to_string(generation) + ".\n");
      terminationReason = "maxFunEvals value reached, final fitness: " + to_string(bestFitness);
      success = false;
      break;
    }
    if (historyConstant) // no entity improved last (mHistorySize) generations
    {
      LOG_SUCC("historyConstant value reached, terminating - generation " + to_string(generation) + ".\n");
      terminationReason = "historyConstant value reached, final fitness: " + to_string(bestFitness);
      success = false;
      break;
    }
  } // generation cycle end

  if (logPoints)
    visitedPoints.push_back(visitedPointsThisRun);

  file << "Evolution ended.\n" << std::endl;
  return bestEntity;
} // optimize function end
