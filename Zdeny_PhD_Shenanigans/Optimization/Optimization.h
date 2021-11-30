#pragma once
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Log/logger.h"

class OptimizationAlgorithm
{
public:
  enum TerminationReason
  {
    NotTerminated = 0,
    OptimalFitnessReached,
    MaximumGenerationsReached,
    MaximumFunctionEvaluationsReached,
    NoImprovementReachedRel,
    NoImprovementReachedAbs,
    UnexpectedErrorOccured,
  };

  using ObjectiveFunction = const std::function<double(const std::vector<double>&)>&;
  using ValidationFunction = ObjectiveFunction;
  using Optimum = std::vector<double>;

  struct OptimizationResult
  {
    Optimum optimum;
    TerminationReason terminationReason = NotTerminated;
    int functionEvaluations = -1;
  };

  OptimizationAlgorithm(int N, const std::string& optname = "default");
  virtual ~OptimizationAlgorithm() = default;

  virtual OptimizationResult Optimize(
      ObjectiveFunction obj, ValidationFunction valid = [](const std::vector<double>&) { return 0; }) = 0;

  void SetConsoleOutput(bool ConsoleOutput) { mConsoleOutput = ConsoleOutput; }
  void SetPlotOutput(bool PlotOutput) { mPlotOutput = PlotOutput; }
  void SetFileOutputDir(const std::string& dir)
  {
    mOutputFileDir = dir;
    mFileOutput = true;
  }
  void SetPlotObjectiveFunctionLandscape(bool plotObjectiveFunctionLandscape) { mPlotObjectiveFunctionLandscape = plotObjectiveFunctionLandscape; }
  void SetPlotObjectiveFunctionLandscapeIterations(int plotObjectiveFunctionLandscapeIterations) { mPlotObjectiveFunctionLandscapeIterations = plotObjectiveFunctionLandscapeIterations; }
  void SetParameterNames(const std::vector<std::string>& ParameterNames) { mParameterNames = ParameterNames; };
  void SetName(const std::string& optname) { mName = optname; }

  static void PlotObjectiveFunctionLandscape(ObjectiveFunction f, const std::vector<double> baseParams, int iters, int xParamIndex, int yParamIndex, double xmin, double xmax, double ymin, double ymax,
      const std::string& xName, const std::string& yName, const std::string& funName);

  int N = 1;                               // the problem dimension
  std::vector<double> mLB;                 // lower search space bounds
  std::vector<double> mUB;                 // upper search space bounds
  double optimalFitness = -Constants::Inf; // satisfactory function value
  int maxFunEvals = Constants::IntInf;     // maximum # of function evaluations
  int maxGen = 1000;                       // maximum # of algorithm iterations

protected:
  static const char* GetTerminationReasonString(const TerminationReason& reason);

  bool mConsoleOutput = true;
  bool mPlotOutput = true;
  bool mFileOutput = false;
  bool mPlotObjectiveFunctionLandscape = false;
  int mPlotObjectiveFunctionLandscapeIterations = 51;
  std::string mOutputFileDir;
  std::string mName;
  std::ofstream mOutputFile;
  std::vector<std::string> mParameterNames;
};
