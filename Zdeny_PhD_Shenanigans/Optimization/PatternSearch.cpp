#include "stdafx.h"
#include "PatternSearch.h"
#include "Plot/Plot1D.h"

OptimizationAlgorithm::OptimizationResult PatternSearch::Optimize(ObjectiveFunction f)
{
  LOG_INFO(" Optimization started (pattern search)");
  vector<double> boundsRange = upperBounds - lowerBounds;
  double initialStep = vectorMax(boundsRange) / 4;
  multistartCnt = 0;
  // multistart algorithm - initialize global results
  vector<double> topPoint = zerovect(N, 0.);
  double topPointFitness = std::numeric_limits<double>::max();
  // generate all starting points
  vector<vector<double>> mainPointsInitial(multistartMaxCnt, zerovect(N, 0.));
  for (int run = 0; run < multistartMaxCnt; run++)
    for (int indexParam = 0; indexParam < N; indexParam++)
      mainPointsInitial[run][indexParam] = randr(lowerBounds[indexParam], upperBounds[indexParam]); // idk dude

  // multistart pattern search
  volatile bool flag = false;
  int funEvals = 0;
#pragma omp parallel for shared(flag)
  for (int run = 0; run < multistartMaxCnt; run++)
  {
    if (flag)
      continue;

    int funEvalsThisRun = 0;
    // initialize vectors
    double step = initialStep;
    vector<double> mainPoint = mainPointsInitial[run];
    double mainPointFitness = f(mainPoint);
    vector<vector<vector<double>>> pattern;                                                    // N-2-N (N pairs of N-dimensional points)
    vector<vector<double>> patternFitness(N, zerovect(2, std::numeric_limits<double>::max())); // N-2 (N pairs of fitness)
    pattern.resize(N);

    for (int dim = 0; dim < N; dim++)
    {
      pattern[dim].resize(2);
      for (int pm = 0; pm < 2; pm++)
        pattern[dim][pm].resize(N);
    }

    // main search cycle
    for (int generation = 1; generation < 1e8; generation++)
    {
      bool smallerStep = true;
      // asign values to pattern vertices - exploration
      for (int dim = 0; dim < N; dim++)
      {
        for (int pm = 0; pm < 2; pm++)
        {
          pattern[dim][pm] = mainPoint;
          pattern[dim][pm][dim] += pm == 0 ? step : -step;
          pattern[dim][pm][dim] = clampSmooth(pattern[dim][pm][dim], mainPoint[dim], lowerBounds[dim], upperBounds[dim]);

          // evaluate vertices
          patternFitness[dim][pm] = f(pattern[dim][pm]);
          funEvalsThisRun++;

          // select best pattern vertex and replace
          if (patternFitness[dim][pm] < mainPointFitness)
          {
            mainPoint = pattern[dim][pm];
            mainPointFitness = patternFitness[dim][pm];
            smallerStep = false;
            // LOG_DEBUG  "> run "  run  " current best entity fitness: "  patternFitness[dim][pm]  ;
            if (maxExploitCnt > 0)
            {
              double testPointFitness = mainPointFitness;
              for (int exploitCnt = 0; exploitCnt < maxExploitCnt; exploitCnt++)
              {
                vector<double> testPoint = mainPoint;
                testPoint[dim] += pm == 0 ? step : -step;
                testPoint[dim] = clampSmooth(testPoint[dim], mainPoint[dim], lowerBounds[dim], upperBounds[dim]);
                testPointFitness = f(testPoint);
                funEvalsThisRun++;

                if (testPointFitness < mainPointFitness)
                {
                  mainPoint = testPoint;
                  mainPointFitness = testPointFitness;
                  // LOG_DEBUG  "> run "  run  " - exploitation "  exploitCnt  " just improved the fitness: "  testPointFitness  ;
                }
              }
            }
          }
        }
      }

      // no improvement - mainPoint is the best point - lower the step size
      if (smallerStep)
        step *= stepReducer;

      // termination criterions
      if (step < minStep)
      {
#pragma omp critical
        {
          // LOG_DEBUG  "> minStep value reached, terminating - generation "  generation  "."  ;
        }
        break;
      }
      if (mainPointFitness < optimalFitness)
      {
#pragma omp critical
        {
          // LOG_DEBUG  "> optimalFitness value reached, terminating - generation "  generation  "."  ;
        }
        break;
      }
      if (generation == maxGen)
      {
#pragma omp critical
        {
          // LOG_DEBUG  "> maxGen value reached, terminating - generation "  generation  "."  ;
        }
        break;
      }
      if ((funEvals >= maxFunEvals) || (funEvalsThisRun >= maxFunEvals))
      {
#pragma omp critical
        {
          // LOG_DEBUG  "MaxFunEvals value reached, terminating - generation "  generation  "."  ;
          flag = true; // dont do other runs, out of funEvals
        }
        break;
      }
    } // generations end

// multistart result update
#pragma omp critical
    {
      multistartCnt++;
      funEvals += funEvalsThisRun;

      // LOG_DEBUG  "> run "  run  ": ";
      if (mainPointFitness < topPointFitness)
      {
        topPoint = mainPoint;
        topPointFitness = mainPointFitness;

        if (topPointFitness < optimalFitness)
          flag = true; // dont do other runs, fitness goal reached
      }
      else
      {
        // LOG_DEBUG  "- run has ended with no improvement, fitness: "  mainPointFitness  ;
      }
    }
  } // multistart end
  return {topPoint, NoImprovementReached};
} // opt end
