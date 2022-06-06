#include "IPCOptimization.hpp"
#include "IPC.hpp"
#include "IPCMeasure.hpp"
#include "Optimization/Evolution.hpp"
#include "Filtering/Noise.hpp"
#include "PhaseCorrelation.hpp"

void IPCOptimization::Optimize(IPC& ipc, const std::string& trainDirectory, const std::string& testDirectory, f64 maxShiftAbs, f64 noiseStddev,
    i32 iters, f64 testRatio, i32 popSize)
try
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;
  LOG_DEBUG("Optimizing IPC for size [{}, {}]", ipc.mCols, ipc.mRows);

  const auto trainImages = LoadImages<IPC::Float>(trainDirectory);
  const auto testImages = LoadImages<IPC::Float>(testDirectory);

  if (trainImages.empty())
    throw std::runtime_error("Empty training images vector");

  const auto maxShift = cv::Point2d(maxShiftAbs, maxShiftAbs);
  const auto shiftOffset1 = cv::Point2d(0.3, 0.6);
  const auto shiftOffset2 = cv::Point2d(0.1 * ipc.mCols, 0.1 * ipc.mRows);
  const auto trainImagePairs = IPCMeasure::CreateImagePairs(ipc, trainImages, maxShift, shiftOffset1, shiftOffset2, iters, noiseStddev);
  const auto testImagePairs =
      IPCMeasure::CreateImagePairs(ipc, testImages, maxShift, shiftOffset1, shiftOffset2, std::max(testRatio * iters, 1.), noiseStddev);
  const auto obj = CreateObjectiveFunction(ipc, trainImagePairs);
  const auto valid = CreateObjectiveFunction(ipc, testImagePairs);

  // before
  LOG_INFO("Running IPC parameter optimization on a set of {}/{} training/validation images with {}/{} image pairs - each generation, {} {}x{} "
           "IPCshifts will be calculated",
      trainImages.size(), testImages.size(), trainImagePairs.size(), testImagePairs.size(), popSize * trainImagePairs.size() + testImagePairs.size(),
      ipc.mCols, ipc.mRows);
  ShowRandomImagePair(trainImagePairs);
  const auto referenceShifts = GetReferenceShifts(trainImagePairs);
  const auto shiftsPixel = GetPixelShifts(ipc, trainImagePairs);
  const auto shiftsNonit = GetNonIterativeShifts(ipc, trainImagePairs);
  const auto shiftsBefore = GetShifts(ipc, trainImagePairs);
  const auto objBefore = GetAverageAccuracy(referenceShifts, shiftsBefore);
  ShowOptimizationPlots(referenceShifts, shiftsPixel, shiftsNonit, shiftsBefore, {});

  // opt
  const auto optimalParameters = CalculateOptimalParameters(obj, valid, popSize);
  if (optimalParameters.empty())
    throw std::runtime_error("Optimization failed");

  // after
  auto ipcAfter = ipc;
  ApplyOptimalParameters(ipcAfter, optimalParameters);
  const auto shiftsAfter = GetShifts(ipcAfter, trainImagePairs);
  const auto objAfter = GetAverageAccuracy(referenceShifts, shiftsAfter);

  if (objAfter >= objBefore)
  {
    LOG_WARNING("Objective function value not improved ({}%), parameters unchanged", static_cast<i32>((objBefore - objAfter) / objBefore * 100));
    return;
  }

  LOG_SUCCESS("Average improvement: {:.2e} -> {:.2e} ({}%)", objBefore, objAfter, static_cast<i32>((objBefore - objAfter) / objBefore * 100));
  ApplyOptimalParameters(ipc, optimalParameters);
  ShowOptimizationPlots(referenceShifts, shiftsPixel, shiftsNonit, shiftsBefore, shiftsAfter);
  LOG_SUCCESS("Iterative Phase Correlation parameter optimization successful");
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}

void IPCOptimization::Optimize(IPC& ipc, const std::function<f64(const IPC&)>& obj, i32 popSize)
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
    LOG_WARNING("Objective function value not improved ({}%), parameters unchanged", static_cast<i32>((objBefore - objAfter) / objBefore * 100));
    return;
  }

  LOG_SUCCESS("Average improvement: {:.2e} -> {:.2e} ({}%)", objBefore, objAfter, static_cast<i32>((objBefore - objAfter) / objBefore * 100));
  ApplyOptimalParameters(ipc, optimalParameters);
  LOG_SUCCESS("Iterative Phase Correlation parameter optimization successful");
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}

IPC IPCOptimization::CreateIPCFromParams(const IPC& ipc_, const std::vector<f64>& params)
{
  PROFILE_FUNCTION;
  IPC ipc(ipc_.mRows, ipc_.mCols);
  ipc.SetBandpassType(static_cast<IPC::BandpassType>((i32)params[BandpassTypeParameter]));
  ipc.SetBandpassParameters(params[BandpassLParameter], params[BandpassHParameter]);
  ipc.SetInterpolationType(static_cast<IPC::InterpolationType>((i32)params[InterpolationTypeParameter]));
  ipc.SetWindowType(static_cast<IPC::WindowType>((i32)params[WindowTypeParameter]));
  ipc.SetL1WindowType(static_cast<IPC::L1WindowType>((i32)params[L1WindowTypeParameter]));
  ipc.SetL2Usize(params[L2UsizeParameter]);
  ipc.SetL1ratio(params[L1ratioParameter]);
  ipc.SetCrossPowerEpsilon(params[CPepsParameter]);
  return ipc;
}

std::function<f64(const std::vector<f64>&)> IPCOptimization::CreateObjectiveFunction(
    const IPC& ipc_, const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;
  return [&](const std::vector<f64>& params)
  {
    const auto ipc = CreateIPCFromParams(ipc_, params);
    if (std::floor(ipc.GetL2Usize() * ipc.GetL1ratio()) < 3)
      return std::numeric_limits<f64>::max();

    f64 avgerror = 0;
    for (const auto& [image1, image2, shift] : imagePairs)
    {
      avgerror += Magnitude(ipc.Calculate(image1, image2) - shift);
    }
    return avgerror / imagePairs.size();
  };
}

std::function<f64(const std::vector<f64>&)> IPCOptimization::CreateObjectiveFunction(const IPC& ipc_, const std::function<f64(const IPC&)>& obj)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;
  return [&](const std::vector<f64>& params)
  {
    const auto ipc = CreateIPCFromParams(ipc_, params);
    if (std::floor(ipc.GetL2Usize() * ipc.GetL1ratio()) < 3)
      return std::numeric_limits<f64>::max();

    return obj(ipc);
  };
}

std::vector<f64> IPCOptimization::CalculateOptimalParameters(
    const std::function<f64(const std::vector<f64>&)>& obj, const std::function<f64(const std::vector<f64>&)>& valid, i32 popSize)
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
  evo.mLB = {0, -0.5, 0, 0, 0, 21, 0.1, -1e-4, 0};
  evo.mUB = {static_cast<f64>(IPC::BandpassType::BandpassTypeCount) - 1e-8, 0.5, 2.,
      static_cast<f64>(IPC::InterpolationType::InterpolationTypeCount) - 1e-8, static_cast<f64>(IPC::WindowType::WindowTypeCount) - 1e-8, 501, 0.8,
      1e-4, static_cast<f64>(IPC::L1WindowType::L1WindowTypeCount) - 1e-8};
  evo.SetPlotOutput(true);
  evo.SetConsoleOutput(true);
  evo.SetParameterValueToNameFunction("BP", [](f64 val) { return IPC::BandpassType2String(static_cast<IPC::BandpassType>((i32)val)); });
  evo.SetParameterValueToNameFunction("BPL", [](f64 val) { return fmt::format("{:.2f}", val); });
  evo.SetParameterValueToNameFunction("BPH", [](f64 val) { return fmt::format("{:.2f}", val); });
  evo.SetParameterValueToNameFunction("INT", [](f64 val) { return IPC::InterpolationType2String(static_cast<IPC::InterpolationType>((i32)val)); });
  evo.SetParameterValueToNameFunction("WIN", [](f64 val) { return IPC::WindowType2String(static_cast<IPC::WindowType>((i32)val)); });
  evo.SetParameterValueToNameFunction("L2U", [](f64 val) { return fmt::format("{}", static_cast<i32>(val)); });
  evo.SetParameterValueToNameFunction("L1R", [](f64 val) { return fmt::format("{:.2f}", val); });
  evo.SetParameterValueToNameFunction("CPeps", [](f64 val) { return fmt::format("{:.2e}", val); });
  evo.SetParameterValueToNameFunction("L1WIN", [](f64 val) { return IPC::L1WindowType2String(static_cast<IPC::L1WindowType>((i32)val)); });
  return evo.Optimize(obj, valid).optimum;
}

void IPCOptimization::ApplyOptimalParameters(IPC& ipc, const std::vector<f64>& optimalParameters)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  if (optimalParameters.size() < OptimizedParameterCount)
    throw std::runtime_error("Cannot apply optimal parameters - wrong parameter count");

  const auto ipcBefore = ipc;
  ipc = CreateIPCFromParams(ipc, optimalParameters);

  LOG_SUCCESS(
      "Final IPC BandpassType: {} -> {}", IPC::BandpassType2String(ipcBefore.GetBandpassType()), IPC::BandpassType2String(ipc.GetBandpassType()));
  if (ipc.GetBandpassType() != IPC::BandpassType::None)
  {
    LOG_SUCCESS("Final IPC BandpassL: {:.2f} -> {:.2f}", ipcBefore.GetBandpassL(), ipc.GetBandpassL());
    LOG_SUCCESS("Final IPC BandpassH: {:.2f} -> {:.2f}", ipcBefore.GetBandpassH(), ipc.GetBandpassH());
  }
  LOG_SUCCESS("Final IPC InterpolationType: {} -> {}", IPC::InterpolationType2String(ipcBefore.GetInterpolationType()),
      IPC::InterpolationType2String(ipc.GetInterpolationType()));
  LOG_SUCCESS("Final IPC WindowType: {} -> {}", IPC::WindowType2String(ipcBefore.GetWindowType()), IPC::WindowType2String(ipc.GetWindowType()));
  LOG_SUCCESS(
      "Final IPC L1WindowType: {} -> {}", IPC::L1WindowType2String(ipcBefore.GetL1WindowType()), IPC::L1WindowType2String(ipc.GetL1WindowType()));
  LOG_SUCCESS("Final IPC L2Usize: {} -> {}", ipcBefore.GetL2Usize(), ipc.GetL2Usize());
  LOG_SUCCESS("Final IPC L1ratio: {:.2f} -> {:.2f}", ipcBefore.GetL1ratio(), ipc.GetL1ratio());
  LOG_SUCCESS("Final IPC CPeps: {:.2e} -> {:.2e}", ipcBefore.GetCrossPowerEpsilon(), ipc.GetCrossPowerEpsilon());
}

void IPCOptimization::ShowOptimizationPlots(const std::vector<cv::Point2d>& shiftsReference, const std::vector<cv::Point2d>& shiftsPixel,
    const std::vector<cv::Point2d>& shiftsNonit, const std::vector<cv::Point2d>& shiftsBefore, const std::vector<cv::Point2d>& shiftsAfter)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  std::vector<f64> shiftsXReference, shiftsXReferenceError;
  std::vector<f64> shiftsXPixel, shiftsXPixelError;
  std::vector<f64> shiftsXNonit, shiftsXNonitError;
  std::vector<f64> shiftsXBefore, shiftsXBeforeError;
  std::vector<f64> shiftsXAfter, shiftsXAfterError;

  for (usize i = 0; i < shiftsReference.size(); ++i)
  {
    const auto& referenceShift = shiftsReference[i];
    const auto& shiftPixel = shiftsPixel[i];
    const auto& shiftNonit = shiftsNonit[i];
    const auto& shiftBefore = shiftsBefore[i];

    shiftsXReference.push_back(referenceShift.x);
    shiftsXReferenceError.push_back(0.0);

    shiftsXPixel.push_back(shiftPixel.x);
    shiftsXNonit.push_back(shiftNonit.x);
    shiftsXBefore.push_back(shiftBefore.x);

    shiftsXPixelError.push_back(shiftPixel.x - referenceShift.x);
    shiftsXNonitError.push_back(shiftNonit.x - referenceShift.x);
    shiftsXBeforeError.push_back(shiftBefore.x - referenceShift.x);

    if (i < shiftsAfter.size())
    {
      const auto& shiftAfter = shiftsAfter[i];
      shiftsXAfter.push_back(shiftAfter.x);
      shiftsXAfterError.push_back(shiftAfter.x - referenceShift.x);
    }
  }

  PyPlot::Plot("IPCshift", {.x = shiftsXReference,
                               .ys = {shiftsXPixel, shiftsXNonit, shiftsXBefore, shiftsXAfter, shiftsXReference},
                               .xlabel = "reference shift [px]",
                               .ylabel = "calculated shift [px]",
                               .label_ys = {"pc", "pcs", "ipc", "ipc opt", "ref"},
                               .color_ys = {"k", "tab:orange", "m", "tab:green", "tab:blue"}});

  PyPlot::Plot("IPCshift error", {.x = shiftsXReference,
                                     .ys = {shiftsXPixelError, shiftsXNonitError, shiftsXBeforeError, shiftsXAfterError, shiftsXReferenceError},
                                     .xlabel = "reference shift [px]",
                                     .ylabel = "error [px]",
                                     .label_ys = {"pc", "pcs", "ipc", "ipc opt", "ref"},
                                     .color_ys = {"k", "tab:orange", "m", "tab:green", "tab:blue"}});
}

std::vector<cv::Point2d> IPCOptimization::GetShifts(const IPC& ipc, const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs)
{
  PROFILE_FUNCTION;
  std::vector<cv::Point2d> out;
  out.reserve(imagePairs.size());

  for (const auto& [image1, image2, referenceShift] : imagePairs)
    out.push_back(ipc.Calculate(image1, image2));

  return out;
}

std::vector<cv::Point2d> IPCOptimization::GetNonIterativeShifts(
    const IPC& ipc, const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs)
{
  PROFILE_FUNCTION;
  std::vector<cv::Point2d> out;
  out.reserve(imagePairs.size());

  for (const auto& [image1, image2, referenceShift] : imagePairs)
    out.push_back(cv::phaseCorrelate(image1, image2));

  return out;
}

std::vector<cv::Point2d> IPCOptimization::GetPixelShifts(const IPC& ipc, const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs)
{
  PROFILE_FUNCTION;
  std::vector<cv::Point2d> out;
  out.reserve(imagePairs.size());

  for (const auto& [image1, image2, referenceShift] : imagePairs)
    out.push_back(PhaseCorrelation::Calculate(image1, image2));

  return out;
}

std::vector<cv::Point2d> IPCOptimization::GetReferenceShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs)
{
  PROFILE_FUNCTION;
  std::vector<cv::Point2d> out;
  out.reserve(imagePairs.size());

  for (const auto& [image1, image2, referenceShift] : imagePairs)
    out.push_back(referenceShift);

  return out;
}

f64 IPCOptimization::GetAverageAccuracy(const std::vector<cv::Point2d>& shiftsReference, const std::vector<cv::Point2d>& shifts)
{
  PROFILE_FUNCTION;
  if (shiftsReference.size() != shifts.size())
    throw std::runtime_error("Reference shift vector has different size than calculated shift vector");

  f64 avgerror = 0;
  for (usize i = 0; i < shifts.size(); ++i)
  {
    const auto error = shifts[i] - shiftsReference[i];
    avgerror += std::sqrt(error.x * error.x + error.y * error.y);
  }
  return avgerror / shifts.size();
}

void IPCOptimization::ShowRandomImagePair(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION;

  const auto& [img1, img2, shift] = imagePairs[static_cast<usize>(Random::Rand() * imagePairs.size())];
  cv::Mat concat;
  cv::hconcat(img1, img2, concat);
  // Plot2D::Set("Random image pair");
  // Plot2D::SetColorMapType(QCPColorGradient::gpGrayscale);
  // Plot2D::Plot("Random image pair", concat);
}
