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

  struct OptimizationResult
  {
    std::vector<f64> optimum;
    TerminationReason terminationReason = NotTerminated;
    i32 functionEvaluations = -1;
    std::vector<f64> bestFitnessProgress;
    std::vector<std::vector<f64>> bestParametersProgress;
    std::vector<std::vector<f64>> evaluatedParameters;
  };

  using ObjectiveFunction = std::function<f64(const std::vector<f64>&)>;

protected:
  usize N = 1;                                            // the problem dimension
  std::vector<f64> mLowerBounds;                          // lower search space bounds
  std::vector<f64> mUpperBounds;                          // upper search space bounds
  f64 mOptimalFitness = -std::numeric_limits<f64>::max(); // satisfactory function value
  usize mMaxFunEvals = std::numeric_limits<i32>::max();   // maximum # of function evaluations
  usize maxGen = 1000;                                    // maximum # of algorithm iterations
  bool mConsoleOutput = true;
  bool mPlotOutput = true;
  bool mFileOutput = false;
  bool mSaveProgress = false;
  std::string mOutputFileDir;
  std::string mName;
  std::ofstream mOutputFile;
  std::vector<std::string> mParameterNames;
  std::vector<std::function<std::string(f64)>> mParameterValueToNameFunctions;

  static f64 ClampSmooth(f64 x_new, f64 x_prev, f64 clampMin, f64 clampMax)
  {
    return x_new < clampMin ? (x_prev + clampMin) / 2 : x_new > clampMax ? (x_prev + clampMax) / 2 : x_new;
  }

  static const char* GetTerminationReasonString(const TerminationReason& reason);

  usize GetParameterIndex(const std::string& parameterName)
  {
    const auto idx = std::ranges::find(mParameterNames, parameterName);
    if (idx == mParameterNames.end())
      throw std::runtime_error(fmt::format("Unknown parameter '{}'", parameterName));
    return idx - mParameterNames.begin();
  }

public:
  explicit OptimizationAlgorithm(i32 N, const std::string& optname = "default");
  virtual ~OptimizationAlgorithm() = default;

  virtual OptimizationResult Optimize(const ObjectiveFunction& obj, const std::optional<ObjectiveFunction>& valid = std::nullopt) = 0;

  static void PlotObjectiveFunctionLandscape(ObjectiveFunction f, const std::vector<f64>& baseParams, i32 iters, i32 xParamIndex, i32 yParamIndex, f64 xmin, f64 xmax, f64 ymin,
      f64 ymax, const std::string& xName, const std::string& yName, const std::string& funName, const OptimizationResult* optResult = nullptr);

  void SetMaxFunEvals(usize maxFunEvals) { mMaxFunEvals = maxFunEvals; }

  void SetOptimalFitness(f64 optimalFitness) { mOptimalFitness = optimalFitness; }

  void Mute()
  {
    SetConsoleOutput(false);
    SetPlotOutput(false);
    SetFileOutput(false);
  }

  void SetConsoleOutput(bool ConsoleOutput) { mConsoleOutput = ConsoleOutput; }

  void SetPlotOutput(bool PlotOutput) { mPlotOutput = PlotOutput; }

  void SetFileOutput(bool FileOutput) { mFileOutput = FileOutput; }

  void SetFileOutputDir(const std::string& dir)
  {
    mOutputFileDir = dir;
    mFileOutput = true;
  }

  void SetSaveProgress(bool SaveProgress) { mSaveProgress = SaveProgress; }

  void SetParameterNames(const std::vector<std::string>& ParameterNames)
  {
    if (ParameterNames.size() != N)
      throw std::runtime_error(fmt::format("Invalid parameter name count ({}!={})", ParameterNames.size(), N));
    mParameterNames = ParameterNames;
  }

  void SetParameterValueToNameFunction(usize ParameterIndex, const std::function<std::string(f64)>& fun)
  {
    if (ParameterIndex < N)
      mParameterValueToNameFunctions[ParameterIndex] = fun;
  }

  void SetParameterValueToNameFunction(const std::string& ParameterName, const std::function<std::string(f64)>& fun)
  {
    for (usize i = 0; i < N; ++i)
      if (mParameterNames[i] == ParameterName)
        return SetParameterValueToNameFunction(i, fun);

    LOG_WARNING("Parameter name {} not defined, ignoring value -> name function", ParameterName);
  }

  void SetName(const std::string& optname) { mName = optname; }

  void SetLowerBounds(const std::vector<f64>& lowerBounds) { mLowerBounds = lowerBounds; }

  void SetUpperBounds(const std::vector<f64>& upperBounds) { mUpperBounds = upperBounds; }

  void SetBounds(const std::string& parameterName, f64 lower, f64 upper)
  {
    const auto index = GetParameterIndex(parameterName);
    mLowerBounds[index] = lower;
    mUpperBounds[index] = upper;
  }
};
