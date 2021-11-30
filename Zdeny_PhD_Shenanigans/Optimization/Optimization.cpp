#include "stdafx.h"
#include "Optimization.h"

OptimizationAlgorithm::OptimizationAlgorithm(int N, const std::string& optname) : N(N), mName(optname)
{
  mLB = zerovect(N, -1.);
  mUB = zerovect(N, 1.);
}

void OptimizationAlgorithm::PlotObjectiveFunctionLandscape(ObjectiveFunction f, const std::vector<double> baseParams, int iters, int xParamIndex, int yParamIndex, double xmin, double xmax,
    double ymin, double ymax, const std::string& xName, const std::string& yName, const std::string& funName, OptimizationResult* optResult)
{
  LOG_FUNCTION("PlotObjectiveFunctionLandscape");
  int rows = iters;
  int cols = iters;
  Mat landscape = Mat::zeros(rows, cols, CV_32F);
  Mat landscapeLog = Mat::zeros(rows, cols, CV_32F);
  std::atomic<int> progress = 0;

#pragma omp parallel for
  for (int r = 0; r < rows; ++r)
  {
    progress++;
    if ((r % std::max(rows / 20, 1) == 0) || r == (rows - 1))
      LOG_INFO("Drawing objective function landscape ({:.1f}%)", (float)progress / rows * 100);

    for (int c = 0; c < cols; ++c)
    {
      auto parameters = baseParams;
      parameters[xParamIndex] = xmin + (double)c / (cols - 1) * (xmax - xmin);
      parameters[yParamIndex] = ymax - (double)r / (rows - 1) * (ymax - ymin);
      const auto fval = f(parameters);
      landscape.at<float>(r, c) = fval;
      landscapeLog.at<float>(r, c) = std::log(1. + fval);
    }
  }

  double minVal, maxVal, minValLog, maxValLog;
  Point minLoci;
  minMaxLoc(landscape, &minVal, &maxVal, &minLoci);
  minMaxLoc(landscapeLog, &minValLog, &maxValLog);
  static const int landscapeSize = 1000;
  double lanscapeSizeMultiplier = (double)landscapeSize / cols;
  resize(landscape, landscape, Size(landscapeSize, landscapeSize * rows / cols), 0, 0, INTER_LINEAR);
  resize(landscapeLog, landscapeLog, Size(landscapeSize, landscapeSize * rows / cols), 0, 0, INTER_LINEAR);
  rows = landscape.rows;
  cols = landscape.cols;
  Point2f minLocf = minLoci;
  minLocf += {0.5, 0.5};
  minLocf *= lanscapeSizeMultiplier;
  Point minLoc = minLocf;

  static const double pointSizeMultiplier = 0.008;
  static const double pointSizeMultiplierMin = 0.01;
  static const double pointThicknessMultiplier = 0.005;
  static const double lineThicknessMultiplier = 0.002;
  static const double circleSizeMultiplier = 1.8;
  const int pointSize = pointSizeMultiplier * cols;
  const int pointSizeMin = pointSizeMultiplierMin * cols;
  const int pointThickness = pointThicknessMultiplier * cols;
  const int lineThickness = lineThicknessMultiplier * cols;
  const Point pointOffset1(pointSize, pointSize);
  const Point pointOffset2(pointSize, -pointSize);
  const Point pointOffset1Min(pointSizeMin, pointSizeMin);
  const Point pointOffset2Min(pointSizeMin, -pointSizeMin);
  static const double pointColorRangeMultiplierMin = 0.5;
  static const double pointColorRangeMultiplier = 0.85;
  const Scalar pointColor(minVal + pointColorRangeMultiplier * (maxVal - minVal));
  const Scalar pointColorLog(minValLog + pointColorRangeMultiplier * (maxValLog - minValLog));
  const Scalar pointColorMin(minVal + pointColorRangeMultiplierMin * (maxVal - minVal));
  const Scalar pointColorMinLog(minValLog + pointColorRangeMultiplierMin * (maxValLog - minValLog));

  line(landscape, minLoc - pointOffset1Min, minLoc + pointOffset1Min, pointColorMin, pointThickness);
  line(landscape, minLoc - pointOffset2Min, minLoc + pointOffset2Min, pointColorMin, pointThickness);
  circle(landscape, minLoc, circleSizeMultiplier * pointSizeMin, pointColorMin, pointThickness);
  line(landscapeLog, minLoc - pointOffset1Min, minLoc + pointOffset1Min, pointColorMinLog, pointThickness);
  line(landscapeLog, minLoc - pointOffset2Min, minLoc + pointOffset2Min, pointColorMinLog, pointThickness);
  circle(landscapeLog, minLoc, circleSizeMultiplier * pointSizeMin, pointColorMinLog, pointThickness);

  LOG_DEBUG("Objective function landscape parameters: minVal: {}, maxVal: {}", minVal, maxVal);

  if (optResult)
  {
    LOG_DEBUG("Drawing optimization progress ({} points)", optResult->parametersProgress.size());

    for (int i = 0; i < optResult->parametersProgress.size(); i++)
    {
      const double x = optResult->parametersProgress[i][xParamIndex];
      const double y = optResult->parametersProgress[i][yParamIndex];

      Point point;
      point.x = (x - xmin) / (xmax - xmin) * cols;
      point.y = rows - 1 - (y - ymin) / (ymax - ymin) * rows;

      line(landscape, point - pointOffset1, point + pointOffset1, pointColor, pointThickness);
      line(landscape, point - pointOffset2, point + pointOffset2, pointColor, pointThickness);
      line(landscapeLog, point - pointOffset1, point + pointOffset1, pointColorLog, pointThickness);
      line(landscapeLog, point - pointOffset2, point + pointOffset2, pointColorLog, pointThickness);

      if (i > 0)
      {
        double xprev = optResult->parametersProgress[i - 1][xParamIndex];
        double yprev = optResult->parametersProgress[i - 1][yParamIndex];

        Point pointprev;
        pointprev.x = (xprev - xmin) / (xmax - xmin) * cols;
        pointprev.y = rows - 1 - (yprev - ymin) / (ymax - ymin) * rows;

        line(landscape, pointprev, point, pointColor, lineThickness);
        line(landscapeLog, pointprev, point, pointColorLog, lineThickness);
      }
    }
  }

  if (true)
  {
    Plot2D::Set(fmt::format("Objective function landscape: {} raw", funName));
    Plot2D::SetXmin(xmin);
    Plot2D::SetXmax(xmax);
    Plot2D::SetYmin(ymin);
    Plot2D::SetYmax(ymax);
    Plot2D::SetXlabel(xName);
    Plot2D::SetYlabel(yName);
    Plot2D::SetZlabel("obj");
    Plot2D::ShowAxisLabels(true);
    Plot2D::Plot(landscape);
  }

  if (true)
  {
    Plot2D::Set(fmt::format("Objective function landscape: {}", funName));
    Plot2D::SetXmin(xmin);
    Plot2D::SetXmax(xmax);
    Plot2D::SetYmin(ymin);
    Plot2D::SetYmax(ymax);
    Plot2D::SetXlabel(xName);
    Plot2D::SetYlabel(yName);
    Plot2D::SetZlabel("log(1+obj)");
    Plot2D::ShowAxisLabels(true);
    Plot2D::Plot(landscapeLog);
  }
}

const char* OptimizationAlgorithm::GetTerminationReasonString(const TerminationReason& reason)
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
