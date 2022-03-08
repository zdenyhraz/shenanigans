#pragma once

class OptimizationAlgorithm
{
public:
  enum TerminationReason : u8
  {
    NotTerminated = 0,
    OptimalFitnessReached,
    MaximumGenerationsReached,
    MaximumFunctionEvaluationsReached,
    NoImprovementReachedRel,
    NoImprovementReachedAbs,
    UnexpectedErrorOccured,
  };

  using ObjectiveFunction = const std::function<f64(const std::vector<f64>&)>&;
  using ValidationFunction = ObjectiveFunction;

  struct OptimizationResult
  {
    std::vector<f64> optimum;
    TerminationReason terminationReason = NotTerminated;
    i32 functionEvaluations = -1;
    std::vector<f64> bestFitnessProgress;
    std::vector<std::vector<f64>> bestParametersProgress;
    std::vector<std::vector<f64>> evaluatedParameters;
  };

  explicit OptimizationAlgorithm(i32 N, const std::string& optname = "default");
  virtual ~OptimizationAlgorithm() = default;

  virtual OptimizationResult Optimize(
      ObjectiveFunction obj, ValidationFunction valid = [](const std::vector<f64>&) { return 0; }) = 0;

  void Mute()
  {
    SetConsoleOutput(false);
    SetPlotOutput(false);
    SetFileOutput(false);
    SetPlotObjectiveFunctionLandscape(false);
  }
  void SetConsoleOutput(bool ConsoleOutput) { mConsoleOutput = ConsoleOutput; }
  void SetPlotOutput(bool PlotOutput) { mPlotOutput = PlotOutput; }
  void SetFileOutput(bool FileOutput) { mFileOutput = FileOutput; }
  void SetFileOutputDir(const std::string& dir)
  {
    mOutputFileDir = dir;
    mFileOutput = true;
  }
  void SetPlotObjectiveFunctionLandscape(bool PlotObjectiveFunctionLandscape) { mPlotObjectiveFunctionLandscape = PlotObjectiveFunctionLandscape; }
  void SetSaveProgress(bool SaveProgress) { mSaveProgress = SaveProgress; }
  void SetPlotObjectiveFunctionLandscapeIterations(i32 PlotObjectiveFunctionLandscapeIterations) { mPlotObjectiveFunctionLandscapeIterations = PlotObjectiveFunctionLandscapeIterations; }
  void SetParameterNames(const std::vector<std::string>& ParameterNames) { mParameterNames = ParameterNames; };
  void SetParameterValueToNameFunction(usize ParameterIndex, const std::function<std::string(f64)>& fun)
  {
    if (ParameterIndex < N)
      mParameterValueToNameFunctions[ParameterIndex] = fun;
  };
  void SetParameterValueToNameFunction(const std::string& ParameterName, const std::function<std::string(f64)>& fun)
  {
    for (usize i = 0; i < N; ++i)
      if (mParameterNames[i] == ParameterName)
        return SetParameterValueToNameFunction(i, fun);

    LOG_WARNING("Parameter name {} not defined, ignoring value -> name function", ParameterName);
  }

  void SetName(const std::string& optname) { mName = optname; }

  static void PlotObjectiveFunctionLandscape(ObjectiveFunction f, const std::vector<f64>& baseParams, i32 iters, i32 xParamIndex, i32 yParamIndex, f64 xmin, f64 xmax, f64 ymin, f64 ymax,
      const std::string& xName, const std::string& yName, const std::string& funName, const OptimizationResult* optResult = nullptr);

  usize N = 1;                                            // the problem dimension
  std::vector<f64> mLB;                                   // lower search space bounds
  std::vector<f64> mUB;                                   // upper search space bounds
  f64 mOptimalFitness = -std::numeric_limits<f64>::max(); // satisfactory function value
  usize mMaxFunEvals = std::numeric_limits<i32>::max();   // maximum # of function evaluations
  usize maxGen = 1000;                                    // maximum # of algorithm iterations

protected:
  static const char* GetTerminationReasonString(const TerminationReason& reason);
  static f64 ClampSmooth(f64 x_new, f64 x_prev, f64 clampMin, f64 clampMax) { return x_new < clampMin ? (x_prev + clampMin) / 2 : x_new > clampMax ? (x_prev + clampMax) / 2 : x_new; }

  bool mConsoleOutput = true;
  bool mPlotOutput = true;
  bool mFileOutput = false;
  bool mPlotObjectiveFunctionLandscape = false;
  bool mSaveProgress = false;
  usize mPlotObjectiveFunctionLandscapeIterations = 51;
  std::string mOutputFileDir;
  std::string mName;
  std::ofstream mOutputFile;
  std::vector<std::string> mParameterNames;
  std::vector<std::function<std::string(f64)>> mParameterValueToNameFunctions;
};
