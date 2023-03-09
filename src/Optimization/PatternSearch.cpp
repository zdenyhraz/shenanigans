#include "PatternSearch.hpp"

OptimizationAlgorithm::OptimizationResult PatternSearch::Optimize(const ObjectiveFunction& obj, const std::optional<ObjectiveFunction>& valid)
{
  LOG_INFO(" Optimization started (pattern search)");
  std::vector<f64> boundsRange = mUpperBounds - mLowerBounds;
  f64 initialStep = *std::max_element(boundsRange.begin(), boundsRange.end()) / 4;
  multistartCnt = 0;
  // multistart algorithm - initialize global results
  std::vector<f64> topPoint = Zerovect(N, 0.);
  f64 topPointFitness = std::numeric_limits<f64>::max();
  // generate all starting points
  std::vector<std::vector<f64>> mainPointsInitial(multistartMaxCnt, Zerovect(N, 0.));
  for (i32 run = 0; run < multistartMaxCnt; run++)
    for (usize indexParam = 0; indexParam < N; indexParam++)
      mainPointsInitial[run][indexParam] = Random::Rand(mLowerBounds[indexParam], mUpperBounds[indexParam]); // idk dude

  // multistart pattern search
  volatile bool flag = false;
  usize funEvals = 0;
#pragma omp parallel for shared(flag)
  for (i32 run = 0; run < multistartMaxCnt; run++)
  {
    if (flag)
      continue;

    usize funEvalsThisRun = 0;
    // initialize vectors
    f64 step = initialStep;
    std::vector<f64> mainPoint = mainPointsInitial[run];
    f64 mainPointFitness = obj(mainPoint);
    std::vector<std::vector<std::vector<f64>>> pattern;                                            // N-2-N (N pairs of N-dimensional points)
    std::vector<std::vector<f64>> patternFitness(N, Zerovect(2, std::numeric_limits<f64>::max())); // N-2 (N pairs of fitness)
    pattern.resize(N);

    for (usize dim = 0; dim < N; dim++)
    {
      pattern[dim].resize(2);
      for (i32 pm = 0; pm < 2; pm++)
        pattern[dim][pm].resize(N);
    }

    // main search cycle
    for (usize generation = 1; generation < 1e8; generation++)
    {
      bool smallerStep = true;
      // asign values to pattern vertices - exploration
      for (usize dim = 0; dim < N; dim++)
      {
        for (i32 pm = 0; pm < 2; pm++)
        {
          pattern[dim][pm] = mainPoint;
          pattern[dim][pm][dim] += pm == 0 ? step : -step;
          pattern[dim][pm][dim] = ClampSmooth(pattern[dim][pm][dim], mainPoint[dim], mLowerBounds[dim], mUpperBounds[dim]);

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
              f64 testPointFitness = mainPointFitness;
              for (i32 exploitCnt = 0; exploitCnt < maxExploitCnt; exploitCnt++)
              {
                std::vector<f64> testPoint = mainPoint;
                testPoint[dim] += pm == 0 ? step : -step;
                testPoint[dim] = ClampSmooth(testPoint[dim], mainPoint[dim], mLowerBounds[dim], mUpperBounds[dim]);
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
