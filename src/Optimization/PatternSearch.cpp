#include "PatternSearch.h"
#include "Plot/Plot1D.h"

OptimizationAlgorithm_::OptimizationResult PatternSearch::Optimize(ObjectiveFunction obj, ValidationFunction valid)
{
  LOG_INFO(" Optimization started (pattern search)");
  std::vector<double> boundsRange = mUB - mLB;
  double initialStep = vectorMax(boundsRange) / 4;
  multistartCnt = 0;
  // multistart algorithm - initialize global results
  std::vector<double> topPoint = zerovect(N, 0.);
  double topPointFitness = std::numeric_limits<double>::max();
  // generate all starting points
  std::vector<std::vector<double>> mainPointsInitial(multistartMaxCnt, zerovect(N, 0.));
  for (int run = 0; run < multistartMaxCnt; run++)
    for (size_t indexParam = 0; indexParam < N; indexParam++)
      mainPointsInitial[run][indexParam] = randr(mLB[indexParam], mUB[indexParam]); // idk dude

  // multistart pattern search
  volatile bool flag = false;
  size_t funEvals = 0;
#pragma omp parallel for shared(flag)
  for (int run = 0; run < multistartMaxCnt; run++)
  {
    if (flag)
      continue;

    size_t funEvalsThisRun = 0;
    // initialize vectors
    double step = initialStep;
    std::vector<double> mainPoint = mainPointsInitial[run];
    double mainPointFitness = obj(mainPoint);
    std::vector<std::vector<std::vector<double>>> pattern;                                               // N-2-N (N pairs of N-dimensional points)
    std::vector<std::vector<double>> patternFitness(N, zerovect(2, std::numeric_limits<double>::max())); // N-2 (N pairs of fitness)
    pattern.resize(N);

    for (size_t dim = 0; dim < N; dim++)
    {
      pattern[dim].resize(2);
      for (int pm = 0; pm < 2; pm++)
        pattern[dim][pm].resize(N);
    }

    // main search cycle
    for (size_t generation = 1; generation < 1e8; generation++)
    {
      bool smallerStep = true;
      // asign values to pattern vertices - exploration
      for (size_t dim = 0; dim < N; dim++)
      {
        for (int pm = 0; pm < 2; pm++)
        {
          pattern[dim][pm] = mainPoint;
          pattern[dim][pm][dim] += pm == 0 ? step : -step;
          pattern[dim][pm][dim] = clampSmooth(pattern[dim][pm][dim], mainPoint[dim], mLB[dim], mUB[dim]);

          // evaluate vertices
          patternFitness[dim][pm] = obj(pattern[dim][pm]);
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
                std::vector<double> testPoint = mainPoint;
                testPoint[dim] += pm == 0 ? step : -step;
                testPoint[dim] = clampSmooth(testPoint[dim], mainPoint[dim], mLB[dim], mUB[dim]);
                testPointFitness = obj(testPoint);
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
      if (mainPointFitness < mOptimalFitness)
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
      if ((funEvals >= mMaxFunEvals) or (funEvalsThisRun >= mMaxFunEvals))
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

        if (topPointFitness < mOptimalFitness)
          flag = true; // dont do other runs, fitness goal reached
      }
      else
      {
        // LOG_DEBUG  "- run has ended with no improvement, fitness: "  mainPointFitness  ;
      }
    }
  } // multistart end
  OptimizationResult result;
  result.optimum = topPoint;
  return result;
} // opt end
