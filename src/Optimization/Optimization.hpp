#pragma once

class OptimizationAlgorithm
{
public:
  enum TerminationReason : uint8_t
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
    std::vector<double> optimum;
    TerminationReason terminationReason = NotTerminated;
    int functionEvaluations = -1;
    std::vector<double> bestFitnessProgress;
    std::vector<std::vector<double>> bestParametersProgress;
    std::vector<std::vector<double>> evaluatedParameters;
  };

  using ObjectiveFunction = std::function<double(const std::vector<double>&)>;

protected:
  size_t N = 1;                                                 // the problem dimension
  std::vector<double> mLowerBounds;                             // lower search space bounds
  std::vector<double> mUpperBounds;                             // upper search space bounds
  double mOptimalFitness = -std::numeric_limits<double>::max(); // satisfactory function value
  size_t mMaxFunEvals = std::numeric_limits<int>::max();        // maximum # of function evaluations
  size_t maxGen = 1000;                                         // maximum # of algorithm iterations
  bool mConsoleOutput = true;
  bool mPlotOutput = true;
  bool mSaveProgress = false;
  std::string mName;
  std::vector<std::string> mParameterNames;
  std::vector<std::function<std::string(double)>> mParameterValueToNameFunctions;

  static double ClampSmooth(double x_new, double x_prev, double clampMin, double clampMax)
  {
    return x_new < clampMin ? (x_prev + clampMin) / 2 : x_new > clampMax ? (x_prev + clampMax) / 2 : x_new;
  }

  static const char* GetTerminationReasonString(const TerminationReason& reason);

  size_t GetParameterIndex(const std::string& parameterName)
  {
    const auto idx = std::ranges::find(mParameterNames, parameterName);
    if (idx == mParameterNames.end())
      throw std::runtime_error(fmt::format("Unknown parameter '{}'", parameterName));
    return idx - mParameterNames.begin();
  }

public:
  explicit OptimizationAlgorithm(int N, const std::string& optname = "default");
  virtual ~OptimizationAlgorithm() = default;

  virtual OptimizationResult Optimize(const ObjectiveFunction& obj, const std::optional<ObjectiveFunction>& valid = std::nullopt) = 0;

  static void PlotObjectiveFunctionLandscape(ObjectiveFunction f, const std::vector<double>& baseParams, int iters, int xParamIndex, int yParamIndex, double xmin, double xmax,
      double ymin, double ymax, const std::string& xName, const std::string& yName, const std::string& funName, const OptimizationResult* optResult = nullptr);

  void SetMaxFunEvals(size_t maxFunEvals) { mMaxFunEvals = maxFunEvals; }
  void SetOptimalFitness(double optimalFitness) { mOptimalFitness = optimalFitness; }
  void SetConsoleOutput(bool ConsoleOutput) { mConsoleOutput = ConsoleOutput; }
  void SetPlotOutput(bool PlotOutput) { mPlotOutput = PlotOutput; }
  void SetSaveProgress(bool SaveProgress) { mSaveProgress = SaveProgress; }
  void SetName(const std::string& optname) { mName = optname; }
  void SetLowerBounds(const std::vector<double>& lowerBounds) { mLowerBounds = lowerBounds; }
  void SetUpperBounds(const std::vector<double>& upperBounds) { mUpperBounds = upperBounds; }

  void SetParameterNames(const std::vector<std::string>& ParameterNames)
  {
    if (ParameterNames.size() != N)
      throw std::runtime_error(fmt::format("Invalid parameter name count ({}!={})", ParameterNames.size(), N));
    mParameterNames = ParameterNames;
  }

  void SetParameterValueToNameFunction(size_t ParameterIndex, const std::function<std::string(double)>& fun)
  {
    if (ParameterIndex < N)
      mParameterValueToNameFunctions[ParameterIndex] = fun;
  }

  void SetParameterValueToNameFunction(const std::string& ParameterName, const std::function<std::string(double)>& fun)
  {
    for (size_t i = 0; i < N; ++i)
      if (mParameterNames[i] == ParameterName)
        return SetParameterValueToNameFunction(i, fun);

    LOG_WARNING("Parameter name {} not defined, ignoring value -> name function", ParameterName);
  }

  void SetBounds(const std::string& parameterName, double lower, double upper)
  {
    const auto index = GetParameterIndex(parameterName);
    mLowerBounds[index] = lower;
    mUpperBounds[index] = upper;
  }

  void Mute()
  {
    SetConsoleOutput(false);
    SetPlotOutput(false);
  }
};
