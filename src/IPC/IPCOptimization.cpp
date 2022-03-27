#include "IPCOptimization.hpp"
#include "IPC.hpp"
#include "Optimization/Evolution.hpp"
#include "Filtering/Noise.hpp"

void IPCOptimization::Optimize(IPC& ipc, const std::string& trainDirectory, const std::string& testDirectory, f64 maxShift, f64 noiseStddev, i32 itersPerImage, f64 testRatio, i32 populationSize)
try
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("Optimize");
  LOG_DEBUG("Optimizing IPC for size [{}, {}]", ipc.mCols, ipc.mRows);

  const auto trainImages = LoadImages(trainDirectory);
  const auto testImages = LoadImages(testDirectory);

  if (trainImages.empty())
    throw std::runtime_error("Empty training images vector");

  const auto trainImagePairs = CreateImagePairs(ipc, trainImages, maxShift, itersPerImage, noiseStddev);
  const auto testImagePairs = CreateImagePairs(ipc, testImages, maxShift, testRatio * itersPerImage, noiseStddev);
  const auto obj = CreateObjectiveFunction(ipc, trainImagePairs);
  const auto valid = CreateObjectiveFunction(ipc, testImagePairs);

  // before
  LOG_INFO("Running IPC parameter optimization on a set of {}/{} training/validation images with {}/{} image pairs - each generation, {} {}x{} IPCshifts will be calculated", trainImages.size(),
      testImages.size(), trainImagePairs.size(), testImagePairs.size(), populationSize * trainImagePairs.size() + testImagePairs.size(), ipc.mCols, ipc.mRows);
  ShowRandomImagePair(trainImagePairs);
  const auto referenceShifts = GetReferenceShifts(trainImagePairs);
  const auto shiftsPixel = GetPixelShifts(ipc, trainImagePairs);
  const auto shiftsNonit = GetNonIterativeShifts(ipc, trainImagePairs);
  const auto shiftsBefore = GetShifts(ipc, trainImagePairs);
  const auto objBefore = GetAverageAccuracy(referenceShifts, shiftsBefore);
  ShowOptimizationPlots(referenceShifts, shiftsPixel, shiftsNonit, shiftsBefore, {});

  // opt
  const auto optimalParameters = CalculateOptimalParameters(obj, valid, populationSize);
  if (optimalParameters.empty())
    throw std::runtime_error("Optimization failed");

  // after
  auto ipcAfter = ipc;
  ApplyOptimalParameters(ipcAfter, optimalParameters);
  const auto shiftsAfter = GetShifts(ipcAfter, trainImagePairs);
  const auto objAfter = GetAverageAccuracy(referenceShifts, shiftsAfter);
  LOG_INFO("Average pixel accuracy improvement: {:.3f} -> {:.3f} ({}%)", objBefore, objAfter, static_cast<i32>((objBefore - objAfter) / objBefore * 100));

  if (objAfter > objBefore)
  {
    LOG_WARNING("Objective function value not improved, parameters unchanged");
    return;
  }

  ApplyOptimalParameters(ipc, optimalParameters);
  ShowOptimizationPlots(referenceShifts, shiftsPixel, shiftsNonit, shiftsBefore, shiftsAfter);
  LOG_SUCCESS("Iterative Phase Correlation parameter optimization successful");
}
catch (const std::exception& e)
{
  LOG_ERROR("Iterative Phase Correlation parameter optimization error: {}", e.what());
}

void IPCOptimization::Optimize(IPC& ipc, const std::function<f64(const IPC&)>& obj, i32 populationSize)
try
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("Optimize");
  LOG_DEBUG("Optimizing IPC for size [{}, {}]", ipc.mCols, ipc.mRows);

  const auto objf = CreateObjectiveFunction(ipc, obj);
  const auto optimalParameters = CalculateOptimalParameters(objf, nullptr, populationSize);
  if (optimalParameters.empty())
    throw std::runtime_error("Optimization failed");

  const auto objBefore = obj(ipc);
  auto ipcAfter = ipc;
  ApplyOptimalParameters(ipcAfter, optimalParameters);
  const auto objAfter = obj(ipcAfter);
  LOG_INFO("Average improvement: {:.2e} -> {:.2e} ({}%)", objBefore, objAfter, static_cast<i32>((objBefore - objAfter) / objBefore * 100));

  if (objAfter > objBefore)
  {
    LOG_WARNING("Objective function value not improved, parameters unchanged");
    return;
  }

  ApplyOptimalParameters(ipc, optimalParameters);
  LOG_SUCCESS("Iterative Phase Correlation parameter optimization successful");
}
catch (const std::exception& e)
{
  LOG_ERROR("Iterative Phase Correlation parameter optimization error: {}", e.what());
}

void IPCOptimization::PlotObjectiveFunctionLandscape(const IPC& ipc, const std::string& trainDirectory, f64 maxShift, f64 noiseStddev, i32 itersPerImage, i32 iters)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("PlotObjectiveFunctionLandscape");
  const auto trainImages = LoadImages(trainDirectory);
  const auto trainImagePairs = CreateImagePairs(ipc, trainImages, maxShift, itersPerImage, noiseStddev);
  const auto obj = CreateObjectiveFunction(ipc, trainImagePairs);
  const i32 rows = iters;
  const i32 cols = iters;
  cv::Mat landscape(rows, cols, GetMatType<IPC::Float>());
  const f64 xmin = -0.25;
  const f64 xmax = 0.75;
  const f64 ymin = 0.25;
  const f64 ymax = 1.25;
  std::atomic<i32> progress = 0;

#pragma omp parallel for
  for (i32 r = 0; r < rows; ++r)
  {
    for (i32 c = 0; c < cols; ++c)
    {
      LOG_INFO("Calculating objective function landscape ({:.1f}%)", static_cast<f64>(progress) / (rows * cols - 1) * 100);
      std::vector<f64> parameters(OptimizedParameterCount);

      // default
      parameters[BandpassTypeParameter] = static_cast<i32>(ipc.mBPT);
      parameters[BandpassLParameter] = ipc.mBPL;
      parameters[BandpassHParameter] = ipc.mBPH;
      parameters[InterpolationTypeParameter] = static_cast<i32>(ipc.mIntT);
      parameters[WindowTypeParameter] = static_cast<i32>(ipc.mWinT);
      parameters[L2UsizeParameter] = ipc.mL2Usize;
      parameters[L1ratioParameter] = ipc.mL1ratio;

      // modified
      parameters[BandpassLParameter] = xmin + (f64)c / (cols - 1) * (xmax - xmin);
      parameters[BandpassHParameter] = ymin + (f64)r / (rows - 1) * (ymax - ymin);

      landscape.at<IPC::Float>(r, c) = std::log(obj(parameters));
      progress++;
    }
  }

  Plot2D::Set("IPCdebug2D");
  Plot2D::SetXmin(xmin);
  Plot2D::SetXmax(xmax);
  Plot2D::SetYmin(ymin);
  Plot2D::SetYmax(ymax);
  Plot2D::Plot("IPCdebug2D", landscape);
}

std::vector<cv::Mat> IPCOptimization::LoadImages(const std::string& imagesDirectory, f64 cropSizeRatio)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("LoadImages");
  LOG_INFO("Loading images from '{}'...", imagesDirectory);

  if (!std::filesystem::is_directory(imagesDirectory))
    throw std::runtime_error(fmt::format("Directory '{}' is not a valid directory", imagesDirectory));

  std::vector<cv::Mat> images;
  for (const auto& entry : std::filesystem::directory_iterator(imagesDirectory))
  {
    const std::string path = entry.path().string();

    if (not IsImagePath(path))
    {
      LOG_WARNING("Directory contains a non-image file {}", path);
      continue;
    }

    auto image = LoadUnitFloatImage<IPC::Float>(path);
    if (cropSizeRatio < 1.0)
      image = RoiCropMid(image, cropSizeRatio * image.cols, cropSizeRatio * image.rows);
    images.push_back(image);
    LOG_DEBUG("Loaded image {}", path);
  }
  return images;
}

std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>> IPCOptimization::CreateImagePairs(const IPC& ipc, const std::vector<cv::Mat>& images, f64 maxShift, i32 itersPerImage, f64 noiseStddev)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("CreateImagePairs");

  if (maxShift <= 0)
    throw std::runtime_error(fmt::format("Invalid max shift ({})", maxShift));
  if (itersPerImage < 1)
    throw std::runtime_error(fmt::format("Invalid iters per image ({})", itersPerImage));
  if (noiseStddev < 0)
    throw std::runtime_error(fmt::format("Invalid noise stdev ({})", noiseStddev));

  if (const auto badimage = std::find_if(images.begin(), images.end(), [&](const auto& image) { return image.rows < ipc.mRows + maxShift or image.cols < ipc.mCols + maxShift; });
      badimage != images.end())
    throw std::runtime_error(fmt::format("Could not optimize IPC parameters - input image is too small for specified IPC window size & max shift ratio ([{},{}] < [{},{}])", badimage->rows,
        badimage->cols, ipc.mRows + maxShift, ipc.mCols + maxShift));

  std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>> imagePairs;
  imagePairs.reserve(images.size() * itersPerImage);

  for (const auto& image : images)
  {
    for (i32 i = 0; i < itersPerImage; ++i)
    {
      // random shift from a random point
      cv::Point2d shift(Random::Rand(-1., 1.) * maxShift, Random::Rand(-1., 1.) * maxShift);
      cv::Point2i point(std::clamp(Random::Rand() * image.cols, static_cast<f64>(ipc.mCols), static_cast<f64>(image.cols - ipc.mCols)),
          std::clamp(Random::Rand() * image.rows, static_cast<f64>(ipc.mRows), static_cast<f64>(image.rows - ipc.mRows)));
      cv::Mat Tmat = (cv::Mat_<f64>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
      cv::Mat imageShifted;
      cv::warpAffine(image, imageShifted, Tmat, image.size());
      cv::Mat image1 = RoiCrop(image, point.x, point.y, ipc.mCols, ipc.mRows);
      cv::Mat image2 = RoiCrop(imageShifted, point.x, point.y, ipc.mCols, ipc.mRows);

      IPC::ConvertToUnitFloat(image1);
      IPC::ConvertToUnitFloat(image2);

      AddNoise<IPC::Float>(image1, noiseStddev);
      AddNoise<IPC::Float>(image2, noiseStddev);

      imagePairs.push_back({image1, image2, shift});

      if constexpr (0)
      {
        cv::Mat hcct;
        hconcat(image1, image2, hcct);
        Showimg(hcct, fmt::format("IPC optimization pair {}", i));
      }
    }
  }

  std::sort(imagePairs.begin(), imagePairs.end(), [](const auto& a, const auto& b) {
    const auto& [img1a, img2a, shifta] = a;
    const auto& [img1b, img2b, shiftb] = b;
    return shifta.x < shiftb.x;
  });
  return imagePairs;
}

IPC IPCOptimization::CreateIPCFromParams(const IPC& ipc_, const std::vector<f64>& params)
{
  PROFILE_FUNCTION;
  IPC ipc(ipc_.mRows, ipc_.mCols);
  ipc.SetBandpassType(static_cast<IPC::BandpassType>((i32)params[BandpassTypeParameter]));
  ipc.SetBandpassParameters(params[BandpassLParameter], params[BandpassHParameter]);
  ipc.SetInterpolationType(static_cast<IPC::InterpolationType>((i32)params[InterpolationTypeParameter]));
  ipc.SetWindowType(static_cast<IPC::WindowType>((i32)params[WindowTypeParameter]));
  ipc.SetL2Usize(params[L2UsizeParameter]);
  ipc.SetL1ratio(params[L1ratioParameter]);
  ipc.SetCrossPowerEpsilon(params[CPepsParameter]);
  return ipc;
}

std::function<f64(const std::vector<f64>&)> IPCOptimization::CreateObjectiveFunction(const IPC& ipc_, const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("CreateObjectiveFunction");
  return [&](const std::vector<f64>& params) {
    const auto ipc = CreateIPCFromParams(ipc_, params);
    if (std::floor(ipc.GetL2Usize() * ipc.GetL1ratio()) < 3)
      return std::numeric_limits<f64>::max();

    f64 avgerror = 0;
    for (const auto& [image1, image2, shift] : imagePairs)
    {
      const auto error = ipc.Calculate(image1, image2) - shift;
      avgerror += sqrt(error.x * error.x + error.y * error.y);
    }
    return avgerror / imagePairs.size();
  };
}

std::function<f64(const std::vector<f64>&)> IPCOptimization::CreateObjectiveFunction(const IPC& ipc_, const std::function<f64(const IPC&)>& obj)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("CreateObjectiveFunction");
  return [&](const std::vector<f64>& params) {
    const auto ipc = CreateIPCFromParams(ipc_, params);
    if (std::floor(ipc.GetL2Usize() * ipc.GetL1ratio()) < 3)
      return std::numeric_limits<f64>::max();

    return obj(ipc);
  };
}

std::vector<f64> IPCOptimization::CalculateOptimalParameters(const std::function<f64(const std::vector<f64>&)>& obj, const std::function<f64(const std::vector<f64>&)>& valid, i32 populationSize)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("CalculateOptimalParameters");

  if (populationSize < 4)
    throw std::runtime_error(fmt::format("Invalid population size ({})", populationSize));

  Evolution evo(OptimizedParameterCount);
  evo.mNP = populationSize;
  evo.mMutStrat = Evolution::RAND1;
  evo.SetParameterNames({"BP", "BPL", "BPH", "INT", "WIN", "L2U", "L1R", "CPeps"});
  evo.mLB = {0, -0.5, 0, 0, 0, 21, 0.1, -1e-4};
  evo.mUB = {static_cast<f64>(IPC::BandpassType::BandpassTypeCount) - 1e-8, 0.5, 2., static_cast<f64>(IPC::InterpolationType::InterpolationTypeCount) - 1e-8,
      static_cast<f64>(IPC::WindowType::WindowTypeCount) - 1e-8, 501, 0.8, 1e-4};
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
  return evo.Optimize(obj, valid).optimum;
}

void IPCOptimization::ApplyOptimalParameters(IPC& ipc, const std::vector<f64>& optimalParameters)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("ApplyOptimalParameters");

  if (optimalParameters.size() < OptimizedParameterCount)
    throw std::runtime_error("Cannot apply optimal parameters - wrong parameter count");

  const auto ipcBefore = ipc;

  ipc.SetBandpassType(static_cast<IPC::BandpassType>((i32)optimalParameters[BandpassTypeParameter]));
  ipc.SetBandpassParameters(optimalParameters[BandpassLParameter], optimalParameters[BandpassHParameter]);
  ipc.SetInterpolationType(static_cast<IPC::InterpolationType>((i32)optimalParameters[InterpolationTypeParameter]));
  ipc.SetWindowType(static_cast<IPC::WindowType>((i32)optimalParameters[WindowTypeParameter]));
  ipc.SetL2Usize(optimalParameters[L2UsizeParameter]);
  ipc.SetL1ratio(optimalParameters[L1ratioParameter]);
  ipc.SetCrossPowerEpsilon(optimalParameters[CPepsParameter]);

  LOG_SUCCESS("Final IPC BandpassType: {} -> {}", IPC::BandpassType2String(ipcBefore.GetBandpassType()), IPC::BandpassType2String(ipc.GetBandpassType()));
  if (ipc.GetBandpassType() != IPC::BandpassType::None)
  {
    LOG_SUCCESS("Final IPC BandpassL: {:.2f} -> {:.2f}", ipcBefore.GetBandpassL(), ipc.GetBandpassL());
    LOG_SUCCESS("Final IPC BandpassH: {:.2f} -> {:.2f}", ipcBefore.GetBandpassH(), ipc.GetBandpassH());
  }
  LOG_SUCCESS("Final IPC InterpolationType: {} -> {}", IPC::InterpolationType2String(ipcBefore.GetInterpolationType()), IPC::InterpolationType2String(ipc.GetInterpolationType()));
  LOG_SUCCESS("Final IPC WindowType: {} -> {}", IPC::WindowType2String(ipcBefore.GetWindowType()), IPC::WindowType2String(ipc.GetWindowType()));
  LOG_SUCCESS("Final IPC L2Usize: {} -> {}", ipcBefore.GetL2Usize(), ipc.GetL2Usize());
  LOG_SUCCESS("Final IPC L1ratio: {:.2f} -> {:.2f}", ipcBefore.GetL1ratio(), ipc.GetL1ratio());
  LOG_SUCCESS("Final IPC CPeps: {:.2e} -> {:.2e}", ipcBefore.GetCrossPowerEpsilon(), ipc.GetCrossPowerEpsilon());
}

void IPCOptimization::ShowOptimizationPlots(const std::vector<cv::Point2d>& shiftsReference, const std::vector<cv::Point2d>& shiftsPixel, const std::vector<cv::Point2d>& shiftsNonit,
    const std::vector<cv::Point2d>& shiftsBefore, const std::vector<cv::Point2d>& shiftsAfter)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("ShowOptimizationPlots");

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
                               .label_ys = {"pixel", "subpixel", "ipc", "ipc opt", "reference"},
                               .color_ys = {"k", "tab:orange", "m", "tab:green", "tab:blue"}});

  PyPlot::Plot("IPCshift error", {.x = shiftsXReference,
                                     .ys = {shiftsXPixelError, shiftsXNonitError, shiftsXBeforeError, shiftsXAfterError, shiftsXReferenceError},
                                     .xlabel = "reference shift [px]",
                                     .ylabel = "error [px]",
                                     .label_ys = {"pixel", "subpixel", "ipc", "ipc opt", "reference"},
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

std::vector<cv::Point2d> IPCOptimization::GetNonIterativeShifts(const IPC& ipc, const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs)
{
  PROFILE_FUNCTION;
  std::vector<cv::Point2d> out;
  out.reserve(imagePairs.size());

  for (const auto& [image1, image2, referenceShift] : imagePairs)
    out.push_back(ipc.Calculate<{.AccuracyT = IPC::AccuracyType::Subpixel}>(image1, image2));

  return out;
}

std::vector<cv::Point2d> IPCOptimization::GetPixelShifts(const IPC& ipc, const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs)
{
  PROFILE_FUNCTION;
  std::vector<cv::Point2d> out;
  out.reserve(imagePairs.size());

  for (const auto& [image1, image2, referenceShift] : imagePairs)
    out.push_back(ipc.Calculate<{.AccuracyT = IPC::AccuracyType::Pixel}>(image1, image2));

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
  LOG_FUNCTION("ShowRandomImagePair");

  const auto& [img1, img2, shift] = imagePairs[static_cast<usize>(Random::Rand() * imagePairs.size())];
  cv::Mat concat;
  cv::hconcat(img1, img2, concat);
  Plot2D::Set("Random image pair");
  Plot2D::SetColorMapType(QCPColorGradient::gpGrayscale);
  Plot2D::Plot("Random image pair", concat);
}
