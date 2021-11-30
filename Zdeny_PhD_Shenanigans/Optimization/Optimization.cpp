#include "stdafx.h"
#include "Optimization.h"

OptimizationAlgorithm::OptimizationAlgorithm(int N, const std::string& optname) : N(N), mName(optname)
{
  mLB = zerovect(N, -1.);
  mUB = zerovect(N, 1.);
}

void OptimizationAlgorithm::PlotObjectiveFunctionLandscape(ObjectiveFunction f, const std::vector<double> baseParams, int iters, int xParamIndex, int yParamIndex, double xmin, double xmax,
    double ymin, double ymax, const std::string& xName, const std::string& yName, const std::string& funName, const OptimizationResult* optResult)
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
      landscape.at<float>(r, c) = f(parameters);
    }
  }

  double minVal, maxVal;
  Point minLoc;
  minMaxLoc(landscape, &minVal, &maxVal, &minLoc);

  const double logConstant = 1.; // std::max(minVal, 1e-8);
  for (int r = 0; r < rows; ++r)
    for (int c = 0; c < cols; ++c)
      landscapeLog.at<float>(r, c) = std::log(landscape.at<float>(r, c) + logConstant);

  double minValLog, maxValLog;
  minMaxLoc(landscapeLog, &minValLog, &maxValLog);

  if (true)
  {
    static const int landscapeSize = 1000;
    double lanscapeSizeMultiplier = (double)landscapeSize / cols;
    resize(landscape, landscape, Size(landscapeSize, lanscapeSizeMultiplier * rows), 0, 0, INTER_LINEAR);
    resize(landscapeLog, landscapeLog, Size(landscapeSize, landscapeSize * rows / cols), 0, 0, INTER_LINEAR);
    rows = landscape.rows;
    cols = landscape.cols;
    Point2f minLocf = minLoc;
    minLocf += {0.5, 0.5};
    minLocf *= lanscapeSizeMultiplier;
    minLoc = minLocf;
  }

  static const double pointSizeMultiplierMin = 0.01;
  static const double pointSizeMultiplierBest = 0.008;
  static const double pointSizeMultiplierEvaluated = 0.004;
  static const double pointThicknessMultiplier = 0.005;
  static const double lineThicknessMultiplier = 0.003;
  static const double pointColorRangeMultiplierMin = 0.5;
  static const double pointColorRangeMultiplierBest = 0.85;
  static const double pointColorRangeMultiplierEvaluated = 0.65;
  const int pointSizeMin = pointSizeMultiplierMin * cols;
  const int pointSizeBest = pointSizeMultiplierBest * cols;
  const int pointSizeEvaluated = pointSizeMultiplierEvaluated * cols;
  const int pointThickness = pointThicknessMultiplier * cols;
  const int lineThickness = lineThicknessMultiplier * cols;
  const Point pointOffset1Min(pointSizeMin, pointSizeMin);
  const Point pointOffset2Min(pointSizeMin, -pointSizeMin);
  const Scalar pointColorBest(minVal + pointColorRangeMultiplierBest * (maxVal - minVal));
  const Scalar pointColorBestLog(minValLog + pointColorRangeMultiplierBest * (maxValLog - minValLog));
  const Scalar pointColorEvaluated(minVal + pointColorRangeMultiplierEvaluated * (maxVal - minVal));
  const Scalar pointColorEvaluatedLog(minValLog + pointColorRangeMultiplierEvaluated * (maxValLog - minValLog));
  const Scalar pointColorMin(minVal + pointColorRangeMultiplierMin * (maxVal - minVal));
  const Scalar pointColorMinLog(minValLog + pointColorRangeMultiplierMin * (maxValLog - minValLog));

  const auto GetPoint = [&](const std::vector<double>& parameters)
  {
    Point point;
    point.x = (parameters[xParamIndex] - xmin) / (xmax - xmin) * cols;
    point.y = rows - 1 - (parameters[yParamIndex] - ymin) / (ymax - ymin) * rows;
    return point;
  };

  const auto GetParams = [&](Mat& mat, const Point& point)
  {
    double x = xmin + (double)point.x / (cols - 1) * (xmax - xmin);
    double y = ymax - (double)point.y / (rows - 1) * (ymax - ymin);
    return std::make_pair(x, y);
  };

  const auto DrawPoint = [](Mat& mat, const Point& point, const Scalar& color, int size, int thickness)
  {
    const Point pointOffset1(size, size);
    const Point pointOffset2(size, -size);

    line(mat, point - pointOffset1, point + pointOffset1, color, thickness, LINE_AA);
    line(mat, point - pointOffset2, point + pointOffset2, color, thickness, LINE_AA);
  };

  const auto DrawCircle = [](Mat& mat, const Point& point, const Scalar& color, int size, int thickness) { circle(mat, point, size, color, thickness, LINE_AA); };

  const auto DrawLine = [](Mat& mat, const Point& point1, const Point& point2, const Scalar& color, int thickness)
  {
    line(mat, point1, point2, color, thickness, LINE_AA);
    // arrowedLine(mat, point1, point2, color, thickness, LINE_AA);
  };

  const auto DrawCircledPoint = [&](Mat& mat, const Point& point, const Scalar& color, int size, int thickness)
  {
    DrawPoint(mat, point, color, size, thickness);
    DrawCircle(mat, point, color, 1.9 * size, thickness);
  };

  const auto [minxcoord, minycoord] = GetParams(landscape, minLoc);
  LOG_DEBUG("Objective function landscape parameters: minVal: {} ({}), maxVal: {}", minVal, Point(minxcoord, minycoord), maxVal);
  LOG_DEBUG("Objective function landscape parameters: minValLog: {} ({}), maxValLog: {}", minValLog, Point(minxcoord, minycoord), maxValLog);

  if (optResult)
  {
    LOG_DEBUG("Drawing optimization progress ({} points)", optResult->bestParametersProgress.size());

    for (const auto& parameters : optResult->evaluatedParameters)
    {
      // all evaluated points
      const Point point = GetPoint(parameters);
      DrawCircle(landscape, point, pointColorEvaluated, pointSizeEvaluated, -1);
      DrawCircle(landscapeLog, point, pointColorEvaluatedLog, pointSizeEvaluated, -1);
    }

    Point pointprev;
    for (int i = 0; i < optResult->bestParametersProgress.size(); i++)
    {
      // best entity points
      const Point point = GetPoint(optResult->bestParametersProgress[i]);
      DrawCircle(landscape, point, pointColorBest, pointSizeBest, -1);
      DrawCircle(landscapeLog, point, pointColorBestLog, pointSizeBest, -1);

      if (i > 0)
      {
        // also draw best path
        if (point != pointprev)
        {
          DrawLine(landscape, pointprev, point, pointColorBest, lineThickness);
          DrawLine(landscapeLog, pointprev, point, pointColorBestLog, lineThickness);
          pointprev = point;
        }
      }
      else
        pointprev = point;
    }
  }

  DrawCircledPoint(landscape, minLoc, pointColorMin, pointSizeMin, pointThickness);
  DrawCircledPoint(landscapeLog, minLoc, pointColorMinLog, pointSizeMin, pointThickness);

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
    Plot2D::SetZlabel(fmt::format("log({:.1e}+obj)", logConstant));
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
