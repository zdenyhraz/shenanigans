#include "stdafx.h"
#include "Optimization.h"

OptimizationAlgorithm::OptimizationAlgorithm(int N) : N(N)
{
  mLB = zerovect(N, -1.);
  mUB = zerovect(N, 1.);
}

void OptimizationAlgorithm::PlotObjectiveFunctionLandscape(ObjectiveFunction f, const std::vector<double> baseParams, int iters, int xParamIndex, int yParamIndex, double xmin, double xmax,
    double ymin, double ymax, const std::string& xName, const std::string& yName, bool applyLog)
{
  LOG_FUNCTION("PlotObjectiveFunctionLandscape");
  int rows = iters;
  int cols = iters;
  auto landscape = zerovect2(rows, cols, 0.0);
  std::atomic<int> progress = 0;

#pragma omp parallel for
  for (int r = 0; r < rows; ++r)
  {
    LOG_INFO("Calculating objective function landscape ({:.1f}%)", (float)progress++ / (rows - 1) * 100);
    for (int c = 0; c < cols; ++c)
    {
      auto parameters = baseParams;
      parameters[xParamIndex] = xmin + (double)c / (cols - 1) * (xmax - xmin);
      parameters[yParamIndex] = ymax - (double)r / (rows - 1) * (ymax - ymin);
      landscape[r][c] = applyLog ? std::log(f(parameters)) : f(parameters);
    }
  }

  Plot2D::Set("ObjectiveFunctionLandscape");
  Plot2D::SetXmin(xmin);
  Plot2D::SetXmax(xmax);
  Plot2D::SetYmin(ymin);
  Plot2D::SetYmax(ymax);
  Plot2D::SetXlabel(xName);
  Plot2D::SetYlabel(yName);
  Plot2D::SetZlabel(applyLog ? "log(obj)" : "obj");
  Plot2D::ShowAxisLabels(true);
  Plot2D::Plot(landscape);
}

std::string OptimizationAlgorithm::GetTerminationReasonString(const TerminationReason& reason)
{
  switch (reason)
  {
  case NotTerminated:
    return "Not yet terminated";
  case OptimalFitnessReached:
    return "Optimal fitness reached";
  case MaximumGenerationsReached:
    return "Maximum generations reached";
  case MaximumFunctionEvaluationsReached:
    return "Maximum function evaluations reached";
  case NoImprovementReachedRel:
    return "No relative improvement reached";
  case NoImprovementReachedAbs:
    return "No absolute improvement reached";
  case UnexpectedErrorOccured:
    return "Unexpected error occured";
  }
  return "Unknown reason";
};
