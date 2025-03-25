#include "IPCOptimization.hpp"
#include "IPC.hpp"
#include "Optimization/Evolution.hpp"
#include "ImageProcessing/Noise.hpp"
#include "PhaseCorrelation.hpp"
#include "ImageRegistrationDataset.hpp"

void IPCOptimization::Optimize(IPC& ipc, const std::string& path, int popSize)
try
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;
  LOG_DEBUG("Optimizing IPC for size [{}, {}]", ipc.mCols, ipc.mRows);

  const auto dataset = LoadImageRegistrationDataset(path);
  const auto obj = [&dataset](const IPC& ipc_)
  {
    if (std::floor(ipc_.GetL2Usize() * ipc_.GetL1ratio()) < 3)
      return std::numeric_limits<double>::max();

    double avgerror = 0;
    for (const auto& imagePair : dataset.imagePairs)
    {
      avgerror += Magnitude(ipc_.Calculate(imagePair.image1, imagePair.image2) - imagePair.shift);
    }
    return avgerror / dataset.imagePairs.size();
  };

  return Optimize(ipc, obj, popSize);
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}

void IPCOptimization::Optimize(IPC& ipc, const std::function<double(const IPC&)>& obj, int popSize)
try
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;
  LOG_DEBUG("Optimizing IPC for size [{}, {}]", ipc.mCols, ipc.mRows);

  const auto optimalParameters = CalculateOptimalParameters(CreateObjectiveFunction(ipc, obj), nullptr, popSize);
  if (optimalParameters.empty())
    throw std::runtime_error("Optimization failed");

  const auto objBefore = obj(ipc);
  auto ipcAfter = ipc;
  ApplyOptimalParameters(ipcAfter, optimalParameters);
  const auto objAfter = obj(ipcAfter);

  if (objAfter >= objBefore)
  {
    LOG_WARNING("Objective function value not improved ({}%), parameters unchanged", static_cast<int>((objBefore - objAfter) / objBefore * 100));
    return;
  }

  ipc = ipcAfter;
  LOG_SUCCESS("Average improvement: {:.2e} -> {:.2e} ({}%)", objBefore, objAfter, static_cast<int>((objBefore - objAfter) / objBefore * 100));
  LOG_SUCCESS("Iterative Phase Correlation parameter optimization successful");
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}

IPC IPCOptimization::CreateIPCFromParams(const IPC& ipc_, const std::vector<double>& params)
{
  PROFILE_FUNCTION;
  IPC ipc(ipc_.mRows, ipc_.mCols);
  ipc.SetBandpassType(static_cast<IPC::BandpassType>((int)params[BandpassTypeParameter]));
  ipc.SetBandpassParameters(params[BandpassLParameter], params[BandpassHParameter]);
  ipc.SetInterpolationType(static_cast<IPC::InterpolationType>((int)params[InterpolationTypeParameter]));
  ipc.SetWindowType(static_cast<IPC::WindowType>((int)params[WindowTypeParameter]));
  ipc.SetL1WindowType(static_cast<IPC::L1WindowType>((int)params[L1WindowTypeParameter]));
  ipc.SetL2Usize(params[L2UsizeParameter]);
  ipc.SetL1ratio(params[L1ratioParameter]);
  ipc.SetCrossPowerEpsilon(params[CPepsParameter]);
  return ipc;
}

std::function<double(const std::vector<double>&)> IPCOptimization::CreateObjectiveFunction(const IPC& ipc_, const std::function<double(const IPC&)>& obj)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;
  return [&](const std::vector<double>& params)
  {
    const auto ipc = CreateIPCFromParams(ipc_, params);
    if (std::floor(ipc.GetL2Usize() * ipc.GetL1ratio()) < 3)
      return std::numeric_limits<double>::max();

    return obj(ipc);
  };
}

std::vector<double> IPCOptimization::CalculateOptimalParameters(
    const std::function<double(const std::vector<double>&)>& obj, const std::function<double(const std::vector<double>&)>& valid, int popSize)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  if (popSize < 4)
    throw std::runtime_error(fmt::format("Invalid population size ({})", popSize));

  Evolution evo(OptimizedParameterCount);
  evo.mNP = popSize;
  evo.mMutStrat = Evolution::BEST1;
  evo.SetName("IPC");
  evo.SetParameterNames({"BP", "BPL", "BPH", "INT", "WIN", "L2U", "L1R", "CPeps", "L1WIN"});
  evo.SetLowerBounds({0, -0.5, 0, 0, 0, 21, 0.1, -1e-4, 0});
  evo.SetUpperBounds({static_cast<double>(IPC::BandpassType::BandpassTypeCount) - 1e-8, 0.5, 2., static_cast<double>(IPC::InterpolationType::InterpolationTypeCount) - 1e-8,
      static_cast<double>(IPC::WindowType::WindowTypeCount) - 1e-8, 501, 0.8, 1e-4, static_cast<double>(IPC::L1WindowType::L1WindowTypeCount) - 1e-8});
  evo.SetPlotOutput(true);
  evo.SetConsoleOutput(true);
  evo.SetParameterValueToNameFunction("BP", [](double val) { return IPC::BandpassType2String(static_cast<IPC::BandpassType>((int)val)); });
  evo.SetParameterValueToNameFunction("BPL", [](double val) { return fmt::format("{:.2f}", val); });
  evo.SetParameterValueToNameFunction("BPH", [](double val) { return fmt::format("{:.2f}", val); });
  evo.SetParameterValueToNameFunction("INT", [](double val) { return IPC::InterpolationType2String(static_cast<IPC::InterpolationType>((int)val)); });
  evo.SetParameterValueToNameFunction("WIN", [](double val) { return IPC::WindowType2String(static_cast<IPC::WindowType>((int)val)); });
  evo.SetParameterValueToNameFunction("L2U", [](double val) { return fmt::format("{}", static_cast<int>(val)); });
  evo.SetParameterValueToNameFunction("L1R", [](double val) { return fmt::format("{:.2f}", val); });
  evo.SetParameterValueToNameFunction("CPeps", [](double val) { return fmt::format("{:.2e}", val); });
  evo.SetParameterValueToNameFunction("L1WIN", [](double val) { return IPC::L1WindowType2String(static_cast<IPC::L1WindowType>((int)val)); });
  return evo.Optimize(obj, valid).optimum;
}

void IPCOptimization::ApplyOptimalParameters(IPC& ipc, const std::vector<double>& optimalParameters)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  if (optimalParameters.size() < OptimizedParameterCount)
    throw std::runtime_error("Cannot apply optimal parameters - wrong parameter count");

  const auto ipcBefore = ipc;
  ipc = CreateIPCFromParams(ipc, optimalParameters);

  LOG_SUCCESS("Final IPC BandpassType: {} -> {}", IPC::BandpassType2String(ipcBefore.GetBandpassType()), IPC::BandpassType2String(ipc.GetBandpassType()));
  if (ipc.GetBandpassType() != IPC::BandpassType::None)
  {
    LOG_SUCCESS("Final IPC BandpassL: {:.2f} -> {:.2f}", ipcBefore.GetBandpassL(), ipc.GetBandpassL());
    LOG_SUCCESS("Final IPC BandpassH: {:.2f} -> {:.2f}", ipcBefore.GetBandpassH(), ipc.GetBandpassH());
  }
  LOG_SUCCESS("Final IPC InterpolationType: {} -> {}", IPC::InterpolationType2String(ipcBefore.GetInterpolationType()), IPC::InterpolationType2String(ipc.GetInterpolationType()));
  LOG_SUCCESS("Final IPC WindowType: {} -> {}", IPC::WindowType2String(ipcBefore.GetWindowType()), IPC::WindowType2String(ipc.GetWindowType()));
  LOG_SUCCESS("Final IPC L1WindowType: {} -> {}", IPC::L1WindowType2String(ipcBefore.GetL1WindowType()), IPC::L1WindowType2String(ipc.GetL1WindowType()));
  LOG_SUCCESS("Final IPC L2Usize: {} -> {}", ipcBefore.GetL2Usize(), ipc.GetL2Usize());
  LOG_SUCCESS("Final IPC L1ratio: {:.2f} -> {:.2f}", ipcBefore.GetL1ratio(), ipc.GetL1ratio());
  LOG_SUCCESS("Final IPC CPeps: {:.2e} -> {:.2e}", ipcBefore.GetCrossPowerEpsilon(), ipc.GetCrossPowerEpsilon());
}
