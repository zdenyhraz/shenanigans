#pragma once
#include "IPCMeasure.hpp"

class IPC;

class IPCOptimization
{
public:
  enum OptimizedParameters : u8
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

  static void Optimize(IPC& ipc, const std::string& path, i32 popSize = 42);
  static void Optimize(IPC& ipc, const std::function<f64(const IPC&)>& obj, i32 popSize = 42);

private:
  static IPC CreateIPCFromParams(const IPC& ipc, const std::vector<f64>& params);
  static std::function<f64(const std::vector<f64>&)> CreateObjectiveFunction(const IPC& ipc, const std::function<f64(const IPC&)>& obj);
  static std::vector<f64> CalculateOptimalParameters(const std::function<f64(const std::vector<f64>&)>& obj, const std::function<f64(const std::vector<f64>&)>& valid, i32 popSize);
  static void ApplyOptimalParameters(IPC& ipc, const std::vector<f64>& optimalParameters);
};
