#pragma once
#include "IPCMeasure.hpp"

class IPC;

class IPCOptimization
{
public:
  enum OptimizedParameters : uint8_t
  {
    BandpassTypeParameter,
    BandpassLParameter,
    BandpassHParameter,
    InterpolationTypeParameter,
    WindowTypeParameter,
    L2UsizeParameter,
    L1ratioParameter,
    CPepsParameter,
    L1WindowTypeParameter,
    OptimizedParameterCount, // last
  };

  static void Optimize(IPC& ipc, const std::string& path, int popSize = 42);
  static void Optimize(IPC& ipc, const std::function<double(const IPC&)>& obj, int popSize = 42);

private:
  static IPC CreateIPCFromParams(const IPC& ipc, const std::vector<double>& params);
  static std::function<double(const std::vector<double>&)> CreateObjectiveFunction(const IPC& ipc, const std::function<double(const IPC&)>& obj);
  static std::vector<double> CalculateOptimalParameters(
      const std::function<double(const std::vector<double>&)>& obj, const std::function<double(const std::vector<double>&)>& valid, int popSize);
  static void ApplyOptimalParameters(IPC& ipc, const std::vector<double>& optimalParameters);
};
