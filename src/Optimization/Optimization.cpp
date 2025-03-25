
#include "Optimization.hpp"
#include "Math/Functions.hpp"
#include "Plot/Plot.hpp"

OptimizationAlgorithm::OptimizationAlgorithm(int N_, const std::string& optname) : N(N_), mName(optname)
{
  PROFILE_FUNCTION;
  mLowerBounds = Zerovect(N, -1.);
  mUpperBounds = Zerovect(N, 1.);
  mParameterNames = std::vector<std::string>(N, "");
  mParameterValueToNameFunctions = std::vector<std::function<std::string(double)>>(N, nullptr);

  // default, can be overridden by the user
  for (size_t i = 0; i < N; ++i)
  {
    mParameterNames[i] = fmt::format("param{}", i);
    mParameterValueToNameFunctions[i] = [](double val) { return fmt::format("{:.1e}", val); };
  }
}

// cppcheck-suppress unusedFunction
void OptimizationAlgorithm::PlotObjectiveFunctionLandscape(ObjectiveFunction f, const std::vector<double>& baseParams, int iters, int xParamIndex, int yParamIndex, double xmin,
    double xmax, double ymin, double ymax, const std::string& xName, const std::string& yName, const std::string& funName, const OptimizationResult* optResult)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  if (xParamIndex < 0 or yParamIndex < 0 or xParamIndex == yParamIndex)
    throw std::runtime_error("Bad x/y parameter indices");

  if (baseParams.size() <= static_cast<size_t>(std::max(xParamIndex, yParamIndex)))
    throw std::runtime_error(fmt::format("Base parameters size ({}) is too small for x/y indices {}/{}", baseParams.size(), xParamIndex, yParamIndex));

  int rows = iters;
  int cols = iters;
  cv::Mat landscape = cv::Mat(rows, cols, CV_32F);
  cv::Mat landscapeLog = cv::Mat(rows, cols, CV_32F);
  std::atomic<int> progress = 0;
  static constexpr double logConstant = std::numeric_limits<float>::epsilon();

#pragma omp parallel for
  for (int r = 0; r < rows; ++r)
  {
    progress++;
    if ((r % std::max(rows / 20, 1) == 0) or r == (rows - 1))
      LOG_INFO("Drawing objective function landscape ({:.1f}%)", (float)progress / rows * 100);

    for (int c = 0; c < cols; ++c)
    {
      auto parameters = baseParams;
      parameters[xParamIndex] = xmin + (double)c / (cols - 1) * (xmax - xmin);
      parameters[yParamIndex] = ymax - (double)r / (rows - 1) * (ymax - ymin);
      const auto funval = f(parameters);
      landscape.at<float>(r, c) = funval;
      landscapeLog.at<float>(r, c) = std::log(std::max(logConstant + funval, logConstant));
    }
  }

  double minVal, maxVal, minValLog, maxValLog;
  cv::Point minLoc;
  cv::minMaxLoc(landscape, &minVal, &maxVal, &minLoc);
  cv::minMaxLoc(landscapeLog, &minValLog, &maxValLog);

  if (true)
  {
    static constexpr int landscapeSize = 1000;
    double lanscapeSizeMultiplier = (double)landscapeSize / cols;
    cv::resize(landscape, landscape, cv::Size(landscapeSize, lanscapeSizeMultiplier * rows), 0, 0, cv::INTER_LINEAR);
    cv::resize(landscapeLog, landscapeLog, cv::Size(landscapeSize, landscapeSize * rows / cols), 0, 0, cv::INTER_LINEAR);
    rows = landscape.rows;
    cols = landscape.cols;
    cv::Point2f minLocf = minLoc;
    minLocf += {0.5, 0.5};
    minLocf *= lanscapeSizeMultiplier;
    minLoc = minLocf;
  }

  Plot::Plot({.name = fmt::format("Objective function landscape: {} raw surf", funName), .z = landscape, .surf = true});
  Plot::Plot({.name = fmt::format("Objective function landscape: {} log surf", funName), .z = landscapeLog, .surf = true});

  static constexpr double pointSizeMultiplierMin = 0.01;
  static constexpr double pointSizeMultiplierBest = 0.008;
  static constexpr double pointSizeMultiplierEvaluated = 0.004;
  static constexpr double pointThicknessMultiplier = 0.005;
  static constexpr double lineThicknessMultiplier = 0.003;
  static constexpr double pointColorRangeMultiplierMin = 0.5;
  static constexpr double pointColorRangeMultiplierBest = 0.85;
  static constexpr double pointColorRangeMultiplierEvaluated = 0.65;
  const int pointSizeMin = pointSizeMultiplierMin * cols;
  const int pointSizeBest = pointSizeMultiplierBest * cols;
  const int pointSizeEvaluated = pointSizeMultiplierEvaluated * cols;
  const int pointThickness = std::max(pointThicknessMultiplier * cols, 1.);
  const int lineThickness = std::max(lineThicknessMultiplier * cols, 1.);
  const cv::Point pointOffset1Min(pointSizeMin, pointSizeMin);
  const cv::Point pointOffset2Min(pointSizeMin, -pointSizeMin);
  const cv::Scalar pointColorBest(minVal + pointColorRangeMultiplierBest * (maxVal - minVal));
  const cv::Scalar pointColorBestLog(minValLog + pointColorRangeMultiplierBest * (maxValLog - minValLog));
  const cv::Scalar pointColorEvaluated(minVal + pointColorRangeMultiplierEvaluated * (maxVal - minVal));
  const cv::Scalar pointColorEvaluatedLog(minValLog + pointColorRangeMultiplierEvaluated * (maxValLog - minValLog));
  const cv::Scalar pointColorMin(minVal + pointColorRangeMultiplierMin * (maxVal - minVal));
  const cv::Scalar pointColorMinLog(minValLog + pointColorRangeMultiplierMin * (maxValLog - minValLog));

  const auto GetPoint = [&](const std::vector<double>& parameters)
  {
    cv::Point point;
    point.x = (parameters[xParamIndex] - xmin) / (xmax - xmin) * cols;
    point.y = rows - 1 - (parameters[yParamIndex] - ymin) / (ymax - ymin) * rows;
    return point;
  };

  const auto DrawPoint = [](cv::Mat& mat, const cv::Point& point, const cv::Scalar& color, int size, int thickness)
  {
    const cv::Point pointOffset1(size, size);
    const cv::Point pointOffset2(size, -size);

    cv::line(mat, point - pointOffset1, point + pointOffset1, color, thickness, cv::LINE_AA);
    cv::line(mat, point - pointOffset2, point + pointOffset2, color, thickness, cv::LINE_AA);
  };

  const auto DrawCircle = [](cv::Mat& mat, const cv::Point& point, const cv::Scalar& color, int size, int thickness)
  { cv::circle(mat, point, size, color, thickness, cv::LINE_AA); };

  const auto DrawLine = [](cv::Mat& mat, const cv::Point& point1, const cv::Point& point2, const cv::Scalar& color, int thickness)
  {
    cv::line(mat, point1, point2, color, thickness, cv::LINE_AA);
    // arrowedLine(mat, point1, point2, color, thickness, cv::LINE_AA);
  };

  const auto DrawCircledPoint = [&](cv::Mat& mat, const cv::Point& point, const cv::Scalar& color, int size, int thickness)
  {
    DrawPoint(mat, point, color, size, thickness);
    DrawCircle(mat, point, color, 1.9 * size, thickness);
  };

  if (optResult)
  {
    LOG_DEBUG("Drawing optimization progress ({} points)", optResult->bestParametersProgress.size());

    for (const auto& parameters : optResult->evaluatedParameters)
    {
      // all evaluated points
      const cv::Point point = GetPoint(parameters);
      DrawCircle(landscape, point, pointColorEvaluated, pointSizeEvaluated, -1);
      DrawCircle(landscapeLog, point, pointColorEvaluatedLog, pointSizeEvaluated, -1);
    }

    cv::Point pointprev;
    for (size_t i = 0; i < optResult->bestParametersProgress.size(); i++)
    {
      // best entity points
      const cv::Point point = GetPoint(optResult->bestParametersProgress[i]);
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

  Plot::Plot({.name = fmt::format("Objective function landscape: {} raw", funName),
      .z = landscape,
      .xmin = xmin,
      .xmax = xmax,
      .ymin = ymin,
      .ymax = ymax,
      .xlabel = xName,
      .ylabel = yName,
      .zlabel = "obj"});
  Plot::Plot({.name = fmt::format("Objective function landscape: {} log", funName),
      .z = landscapeLog,
      .xmin = xmin,
      .xmax = xmax,
      .ymin = ymin,
      .ymax = ymax,
      .xlabel = xName,
      .ylabel = yName,
      .zlabel = fmt::format("log({:.1e}+obj)", logConstant)});
}

// cppcheck-suppress unusedFunction
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
  default:
    return "Unknown reason";
  }
}
