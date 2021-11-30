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
    if ((r % (rows / 20) == 0) || r == (rows - 1))
      LOG_INFO("Drawing objective function landscape ({:.1f}%)", (float)progress / rows * 100);

    for (int c = 0; c < cols; ++c)
    {
      auto parameters = baseParams;
      parameters[xParamIndex] = xmin + (double)c / (cols - 1) * (xmax - xmin);
      parameters[yParamIndex] = ymax - (double)r / (rows - 1) * (ymax - ymin);
      const auto fval = f(parameters);
      landscape.at<float>(r, c) = fval;
      landscapeLog.at<float>(r, c) = std::log(fval);
    }
  }

  static const int landscapeSize = 1000;
  resize(landscape, landscape, Size(landscapeSize, landscapeSize * rows / cols), 0, 0, INTER_LINEAR);
  resize(landscapeLog, landscapeLog, Size(landscapeSize, landscapeSize * rows / cols), 0, 0, INTER_LINEAR);
  rows = landscape.rows;
  cols = landscape.cols;

  double minVal, maxVal, minValLog, maxValLog;
  Point minLoc;
  minMaxLoc(landscape, &minVal, &maxVal, &minLoc);
  minMaxLoc(landscapeLog, &minValLog, &maxValLog);
  static const double pointSizeMultiplier = 0.01;
  static const double pointThicknessMultiplier = 0.005;
  static const double lineThicknessMultiplier = 0.002;
  const int pointSize = pointSizeMultiplier * cols;
  const int pointThickness = pointThicknessMultiplier * cols;
  const int lineThickness = lineThicknessMultiplier * cols;
  const Point pointOffset1(pointSize, pointSize);
  const Point pointOffset2(pointSize, -pointSize);

  LOG_DEBUG("Objective function landscape parameters: minVal: {:.1e}, maxVal:{:.1e}", minVal, maxVal);

  if (optResult)
  {
    if (true) // dbg
    {
      const int dbgcnt = 51;
      optResult->parametersProgress.clear();
      optResult->parametersProgress.resize(dbgcnt);

      optResult->fitnessProgress.clear();
      optResult->fitnessProgress.resize(dbgcnt);

      for (int i = 0; i < dbgcnt; i++)
      {
        const double dbgX = -4. + ((double)i / (dbgcnt - 1)) * 8.;
        const double dbgY = 4. - ((double)i / (dbgcnt - 1)) * 4.;
        optResult->parametersProgress[i] = baseParams;
        optResult->parametersProgress[i][xParamIndex] = dbgX;
        optResult->parametersProgress[i][yParamIndex] = dbgY;
        optResult->fitnessProgress[i] = f(optResult->parametersProgress[i]);
      }
    }

    LOG_DEBUG("Drawing optimization progress ({} points)", optResult->parametersProgress.size());

    for (int i = 0; i < optResult->parametersProgress.size(); i++)
    {
      const double x = optResult->parametersProgress[i][xParamIndex];
      const double y = optResult->parametersProgress[i][yParamIndex];
      const double fitness = std::clamp(optResult->fitnessProgress[i], minVal, maxVal);
      const double fitnessLog = std::log(fitness);
      static const double pointColorMultiplierMin = 0.0;
      static const double pointColorMultiplierMax = 1.0;
      const double pointColorMultiplier = (std::clamp(1.0 - (fitness - minVal) / (maxVal - minVal), pointColorMultiplierMin, pointColorMultiplierMax));
      const double pointColorMultiplierLog = (std::clamp(1.0 - (fitnessLog - minValLog) / (maxValLog - minValLog), pointColorMultiplierMin, pointColorMultiplierMax));
      const Scalar pointColor(minVal + pointColorMultiplier * (maxVal - minVal));
      const Scalar pointColorLog(minValLog + pointColorMultiplierLog * (maxValLog - minValLog));

      LOG_DEBUG("Drawing optimization progress point {}/{}, fitness: {:.1e}, pointColorMultiplier: {:.1f}, pointColorMultiplierLog: {:.1f}", i + 1, optResult->parametersProgress.size(), fitness,
          pointColorMultiplier, pointColorMultiplierLog);

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
        double fitnessprev = optResult->fitnessProgress[i - 1];
        const double fitnessprevLog = std::log(fitnessprev);
        const double pointColorMultiplierprev = (std::clamp(1.0 - (fitnessprev - minVal) / (maxVal - minVal), pointColorMultiplierMin, pointColorMultiplierMax));
        const double pointColorMultiplierprevLog = (std::clamp(1.0 - (fitnessprevLog - minValLog) / (maxValLog - minValLog), pointColorMultiplierMin, pointColorMultiplierMax));
        const Scalar pointColorprev(minVal + pointColorMultiplierprev * (maxVal - minVal));
        const Scalar pointColorprevLog(minValLog + pointColorMultiplierprevLog * (maxValLog - minValLog));

        Point pointprev;
        pointprev.x = (xprev - xmin) / (xmax - xmin) * cols;
        pointprev.y = rows - 1 - (yprev - ymin) / (ymax - ymin) * rows;

        line(landscape, pointprev, point, pointColorprev, lineThickness);
        line(landscapeLog, pointprev, point, pointColorprevLog, lineThickness);
      }
    }
  }

  static const double pointColorMinMultiplier = 0.5;
  static const double circleSizeMultiplier = 2.2;
  const Scalar pointColorMin(minVal + pointColorMinMultiplier * (maxVal - minVal));
  const Scalar pointColorMinLog(minValLog + pointColorMinMultiplier * (maxValLog - minValLog));
  line(landscape, minLoc - pointOffset1, minLoc + pointOffset1, pointColorMin, pointThickness);
  line(landscape, minLoc - pointOffset2, minLoc + pointOffset2, pointColorMin, pointThickness);
  circle(landscape, minLoc, circleSizeMultiplier * pointSize, pointColorMin, pointThickness);
  line(landscapeLog, minLoc - pointOffset1, minLoc + pointOffset1, pointColorMinLog, pointThickness);
  line(landscapeLog, minLoc - pointOffset2, minLoc + pointOffset2, pointColorMinLog, pointThickness);
  circle(landscapeLog, minLoc, circleSizeMultiplier * pointSize, pointColorMinLog, pointThickness);

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
    Plot2D::SetZlabel("log(obj)");
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
