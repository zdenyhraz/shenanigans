#include "IterativePhaseCorrelation.h"
#include "Filtering/Noise.h"

template class IterativePhaseCorrelation<f32>;
template class IterativePhaseCorrelation<f64>;

template <typename Float>
cv::Mat IterativePhaseCorrelation<Float>::Align(const cv::Mat& image1, const cv::Mat& image2) const
{
  return Align(image1.clone(), image2.clone());
}

template <typename Float>
cv::Mat IterativePhaseCorrelation<Float>::Align(cv::Mat&& image1, cv::Mat&& image2) const
try
{
  PROFILE_FUNCTION;
  cv::Mat img1W = image1.clone();
  cv::Mat img2W = image2.clone();
  ApplyWindow<false>(img1W);
  ApplyWindow<false>(img2W);
  cv::Mat img1FT = Fourier::fft(img1W);
  cv::Mat img2FT = Fourier::fft(img2W);
  Fourier::fftshift(img1FT);
  Fourier::fftshift(img2FT);
  cv::Mat img1FTm = cv::Mat(img1FT.size(), GetMatType<Float>());
  cv::Mat img2FTm = cv::Mat(img2FT.size(), GetMatType<Float>());
  for (i32 row = 0; row < img1FT.rows; ++row)
  {
    auto img1FTp = img1FT.ptr<cv::Vec<Float, 2>>(row);
    auto img2FTp = img2FT.ptr<cv::Vec<Float, 2>>(row);
    auto img1FTmp = img1FTm.ptr<Float>(row);
    auto img2FTmp = img2FTm.ptr<Float>(row);
    for (i32 col = 0; col < img1FT.cols; ++col)
    {
      const auto& re1 = img1FTp[col][0];
      const auto& im1 = img1FTp[col][1];
      const auto& re2 = img2FTp[col][0];
      const auto& im2 = img2FTp[col][1];
      img1FTmp[col] = std::log(std::sqrt(re1 * re1 + im1 * im1));
      img2FTmp[col] = std::log(std::sqrt(re2 * re2 + im2 * im2));
    }
  }
  cv::Point2d center(0.5 * image1.cols, 0.5 * image1.rows);
  f64 maxRadius = std::min(center.y, center.x);
  warpPolar(img1FTm, img1FTm, img1FTm.size(), center, maxRadius, cv::INTER_LINEAR | cv::WARP_FILL_OUTLIERS | cv::WARP_POLAR_LOG); // semilog Polar
  warpPolar(img2FTm, img2FTm, img2FTm.size(), center, maxRadius, cv::INTER_LINEAR | cv::WARP_FILL_OUTLIERS | cv::WARP_POLAR_LOG); // semilog Polar

  // rotation
  auto shiftR = Calculate(img1FTm, img2FTm);
  f64 rotation = -shiftR.y / image1.rows * 360;
  f64 scale = exp(shiftR.x * log(maxRadius) / image1.cols);
  Rotate(image2, -rotation, scale);

  // translation
  auto shiftT = Calculate(image1, image2);
  Shift(image2, -shiftT);

  if constexpr (0)
  {
    LOG_INFO("Evaluated rotation: {} deg", rotation);
    LOG_INFO("Evaluated scale: {}", 1.f / scale);
    LOG_INFO("Evaluated shift: {} px", shiftT);
    Showimg(ColorComposition(image1, image2), "color composition result", 0, 0, 1, 1000);
  }
  return image2;
}
catch (const std::exception& e)
{
  LOG_ERROR("IPC Align error: {}", e.what());
  return cv::Mat();
}

template <typename Float>
std::tuple<cv::Mat, cv::Mat> IterativePhaseCorrelation<Float>::CalculateFlow(const cv::Mat& image1, const cv::Mat& image2, f64 resolution) const
{
  return CalculateFlow(image1.clone(), image2.clone(), resolution);
}

template <typename Float>
std::tuple<cv::Mat, cv::Mat> IterativePhaseCorrelation<Float>::CalculateFlow(cv::Mat&& image1, cv::Mat&& image2, f64 resolution) const
try
{
  PROFILE_FUNCTION;
  if (image1.size() != image2.size())
    throw std::runtime_error(fmt::format("Image sizes differ ({} != {})", image1.size(), image2.size()));

  if (mRows > image1.rows or mCols > image1.cols)
    throw std::runtime_error(fmt::format("Images are too small ({} < {})", image1.size(), cv::Size(mCols, mRows)));

  cv::Mat flowX = cv::Mat(cv::Size(resolution * image1.cols, resolution * image1.rows), GetMatType<Float>());
  cv::Mat flowY = cv::Mat(cv::Size(resolution * image2.cols, resolution * image2.rows), GetMatType<Float>());
  std::atomic<i32> progress = 0;

#pragma omp parallel for
  for (i32 r = 0; r < flowX.rows; ++r)
  {
    if (++progress % (flowX.rows / 20) == 0)
      LOG_DEBUG("Calculating IPC flow profile ({:.0f}%)", static_cast<f64>(progress) / flowX.rows * 100);

    for (i32 c = 0; c < flowX.cols; ++c)
    {
      const cv::Point2i center(c / resolution, r / resolution);

      if (IsOutOfBounds(center, image1, {mCols, mRows}))
        continue;

      const auto shift = Calculate(RoiCrop(image1, center.x, center.y, mCols, mRows), RoiCrop(image2, center.x, center.y, mCols, mRows));
      flowX.at<Float>(r, c) = shift.x;
      flowY.at<Float>(r, c) = shift.y;
    }
  }

  return {flowX, flowY};
}
catch (const std::exception& e)
{
  LOG_ERROR("IPC CalculateFlow error: {}", e.what());
  return {};
}

template <typename Float>
void IterativePhaseCorrelation<Float>::Optimize(
    const std::string& trainingImagesDirectory, const std::string& validationImagesDirectory, f64 maxShift, f64 noiseStdev, i32 itersPerImage, f64 validationRatio, i32 populationSize)
try
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("Optimize");
  LOG_DEBUG("Optimizing IPC for size [{}, {}]", mCols, mRows);

  const auto trainingImages = LoadImages(trainingImagesDirectory);
  const auto validationImages = LoadImages(validationImagesDirectory);

  if (trainingImages.empty())
    throw std::runtime_error("Empty training images vector");

  const auto trainingImagePairs = CreateImagePairs(trainingImages, maxShift, itersPerImage, noiseStdev);
  const auto validationImagePairs = CreateImagePairs(validationImages, maxShift, validationRatio * itersPerImage, noiseStdev);
  const auto obj = CreateObjectiveFunction(trainingImagePairs);
  const auto valid = CreateObjectiveFunction(validationImagePairs);

  // before
  LOG_INFO("Running Iterative Phase Correlation parameter optimization on a set of {}/{} training/validation images with {}/{} image pairs - each generation, {} {}x{} IPCshifts will be calculated",
      trainingImages.size(), validationImages.size(), trainingImagePairs.size(), validationImagePairs.size(), populationSize * trainingImagePairs.size() + validationImagePairs.size(), mCols, mRows);
  ShowRandomImagePair(trainingImagePairs);
  const auto referenceShifts = GetReferenceShifts(trainingImagePairs);
  const auto shiftsPixel = GetPixelShifts(trainingImagePairs);
  const auto shiftsNonit = GetNonIterativeShifts(trainingImagePairs);
  const auto shiftsBefore = GetShifts(trainingImagePairs);
  const auto objBefore = GetAverageAccuracy(referenceShifts, shiftsBefore);
  ShowOptimizationPlots(referenceShifts, shiftsPixel, shiftsNonit, shiftsBefore, {});

  // opt
  const auto optimalParameters = CalculateOptimalParameters(obj, valid, populationSize);
  if (optimalParameters.empty())
    throw std::runtime_error("Optimization failed");

  // after
  auto thisAfter = *this;
  thisAfter.ApplyOptimalParameters(optimalParameters);
  const auto shiftsAfter = thisAfter.GetShifts(trainingImagePairs);
  const auto objAfter = GetAverageAccuracy(referenceShifts, shiftsAfter);
  LOG_INFO("Average pixel accuracy improvement: {:.3f} -> {:.3f} ({}%)", objBefore, objAfter, static_cast<i32>((objBefore - objAfter) / objBefore * 100));

  if (objAfter > objBefore)
  {
    LOG_WARNING("Objective function value not improved, parameters unchanged");
    return;
  }

  ApplyOptimalParameters(optimalParameters);
  ShowOptimizationPlots(referenceShifts, shiftsPixel, shiftsNonit, shiftsBefore, shiftsAfter);
  LOG_SUCCESS("Iterative Phase Correlation parameter optimization successful");
}
catch (const std::exception& e)
{
  LOG_ERROR("Iterative Phase Correlation parameter optimization error: {}", e.what());
}

template <typename Float>
void IterativePhaseCorrelation<Float>::Optimize(const std::function<f64(const IterativePhaseCorrelation&)>& obj, i32 populationSize)
try
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("Optimize");
  LOG_DEBUG("Optimizing IPC for size [{}, {}]", mCols, mRows);

  const auto objf = CreateObjectiveFunction(obj);
  const auto optimalParameters = CalculateOptimalParameters(objf, nullptr, populationSize);
  if (optimalParameters.empty())
    throw std::runtime_error("Optimization failed");

  const auto objBefore = obj(*this);
  auto thisAfter = *this;
  thisAfter.ApplyOptimalParameters(optimalParameters);
  const auto objAfter = obj(thisAfter);
  LOG_INFO("Average improvement: {:.2e} -> {:.2e} ({}%)", objBefore, objAfter, static_cast<i32>((objBefore - objAfter) / objBefore * 100));

  if (objAfter > objBefore)
  {
    LOG_WARNING("Objective function value not improved, parameters unchanged");
    return;
  }

  ApplyOptimalParameters(optimalParameters);
  LOG_SUCCESS("Iterative Phase Correlation parameter optimization successful");
}
catch (const std::exception& e)
{
  LOG_ERROR("Iterative Phase Correlation parameter optimization error: {}", e.what());
}

template <typename Float>
void IterativePhaseCorrelation<Float>::PlotObjectiveFunctionLandscape(const std::string& trainingImagesDirectory, f64 maxShift, f64 noiseStdev, i32 itersPerImage, i32 iters) const
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("PlotObjectiveFunctionLandscape");
  const auto trainingImages = LoadImages(trainingImagesDirectory);
  const auto trainingImagePairs = CreateImagePairs(trainingImages, maxShift, itersPerImage, noiseStdev);
  const auto obj = CreateObjectiveFunction(trainingImagePairs);
  const i32 rows = iters;
  const i32 cols = iters;
  cv::Mat landscape(rows, cols, GetMatType<Float>());
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
      parameters[BandpassTypeParameter] = static_cast<i32>(mBandpassType);
      parameters[BandpassLParameter] = mBandpassL;
      parameters[BandpassHParameter] = mBandpassH;
      parameters[InterpolationTypeParameter] = static_cast<i32>(mInterpolationType);
      parameters[WindowTypeParameter] = static_cast<i32>(mWindowType);
      parameters[UpsampleCoeffParameter] = mUpsampleCoeff;
      parameters[L1ratioParameter] = mL1ratio;

      // modified
      parameters[BandpassLParameter] = xmin + (f64)c / (cols - 1) * (xmax - xmin);
      parameters[BandpassHParameter] = ymin + (f64)r / (rows - 1) * (ymax - ymin);

      landscape.at<Float>(r, c) = std::log(obj(parameters));
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

template <typename Float>
void IterativePhaseCorrelation<Float>::PlotImageSizeAccuracyDependence(const std::string& trainingImagesDirectory, f64 maxShift, f64 noiseStdev, i32 itersPerImage, i32 iters)
{
  LOG_FUNCTION("PlotImageSizeAccuracyDependence");

  const auto trainingImages = LoadImages(trainingImagesDirectory);
  std::vector<f64> imageSizes(iters);
  std::vector<f64> accuracy(iters);
  const f64 xmin = 16;
  const f64 xmax = mRows;
  std::atomic<i32> progress = 0;

  for (i32 i = 0; i < iters; ++i)
  {
    LOG_INFO("Calculating image size accuracy dependence ({:.1f}%)", static_cast<f64>(progress) / (iters - 1) * 100);
    i32 imageSize = xmin + (f64)i / (iters - 1) * (xmax - xmin);
    imageSize = imageSize % 2 ? imageSize + 1 : imageSize;
    SetSize(imageSize, imageSize);
    const auto trainingImagePairs = CreateImagePairs(trainingImages, maxShift, itersPerImage, noiseStdev);
    const auto obj = CreateObjectiveFunction(trainingImagePairs);
    std::vector<f64> parameters(OptimizedParameterCount);

    // default
    parameters[BandpassTypeParameter] = static_cast<i32>(mBandpassType);
    parameters[BandpassLParameter] = mBandpassL;
    parameters[BandpassHParameter] = mBandpassH;
    parameters[InterpolationTypeParameter] = static_cast<i32>(mInterpolationType);
    parameters[WindowTypeParameter] = static_cast<i32>(mWindowType);
    parameters[UpsampleCoeffParameter] = mUpsampleCoeff;
    parameters[L1ratioParameter] = mL1ratio;

    imageSizes[i] = imageSize;
    accuracy[i] = obj(parameters);
    progress++;
  }

  Plot1D::Set("ImageSizeAccuracyDependence");
  Plot1D::SetXlabel("Image size");
  Plot1D::SetYlabel("Average pixel error");
  // Plot1D::SetYLogarithmic(true);
  Plot1D::Plot("ImageSizeAccuracyDependence", imageSizes, accuracy);
}

template <typename Float>
void IterativePhaseCorrelation<Float>::PlotUpsampleCoefficientAccuracyDependence(const std::string& trainingImagesDirectory, f64 maxShift, f64 noiseStdev, i32 itersPerImage, i32 iters) const
{
  LOG_FUNCTION("PlotUpsampleCoefficientAccuracyDependence");

  const auto trainingImages = LoadImages(trainingImagesDirectory);
  const auto trainingImagePairs = CreateImagePairs(trainingImages, maxShift, itersPerImage, noiseStdev);
  const auto obj = CreateObjectiveFunction(trainingImagePairs);

  std::vector<f64> upsampleCoeff(iters);
  std::vector<f64> accuracy(iters);
  const f64 xmin = 1;
  const f64 xmax = 35;
  std::atomic<i32> progress = 0;

  for (i32 i = 0; i < iters; ++i)
  {
    LOG_INFO("Calculating upsample coeffecient accuracy dependence ({:.1f}%)", static_cast<f64>(progress) / (iters - 1) * 100);
    std::vector<f64> parameters(OptimizedParameterCount);

    // default
    parameters[BandpassTypeParameter] = static_cast<i32>(mBandpassType);
    parameters[BandpassLParameter] = mBandpassL;
    parameters[BandpassHParameter] = mBandpassH;
    parameters[InterpolationTypeParameter] = static_cast<i32>(mInterpolationType);
    parameters[WindowTypeParameter] = static_cast<i32>(mWindowType);
    parameters[UpsampleCoeffParameter] = mUpsampleCoeff;
    parameters[L1ratioParameter] = mL1ratio;

    // modified
    parameters[UpsampleCoeffParameter] = xmin + (f64)i / (iters - 1) * (xmax - xmin);

    upsampleCoeff[i] = parameters[UpsampleCoeffParameter];
    accuracy[i] = obj(parameters);
    progress++;
  }

  Plot1D::Set("UpsampleCoefficientAccuracyDependence");
  Plot1D::SetXlabel("Upsample coefficient");
  Plot1D::SetYlabel("Average pixel error");
  Plot1D::SetYLogarithmic(true);
  Plot1D::Plot("UpsampleCoefficientAccuracyDependence", upsampleCoeff, accuracy);
}

template <typename Float>
void IterativePhaseCorrelation<Float>::PlotNoiseAccuracyDependence(const std::string& trainingImagesDirectory, f64 maxShift, f64 noiseStdev, i32 itersPerImage, i32 iters) const
{
  LOG_FUNCTION("PlotNoiseAccuracyDependence");

  if (noiseStdev <= 0.0f)
  {
    LOG_ERROR("Please set some non-zero positive max noise stdev");
    return;
  }

  const auto trainingImages = LoadImages(trainingImagesDirectory);
  std::vector<f64> noiseStdevs(iters);
  std::vector<f64> accuracy(iters);
  const f64 xmin = 0;
  const f64 xmax = noiseStdev;
  std::atomic<i32> progress = 0;

#pragma omp parallel for
  for (i32 i = 0; i < iters; ++i)
  {
    LOG_INFO("Calculating noise stdev accuracy dependence ({:.1f}%)", static_cast<f64>(progress) / (iters - 1) * 100);
    f64 noise = xmin + static_cast<f64>(i) / (iters - 1) * (xmax - xmin);
    const auto trainingImagePairs = CreateImagePairs(trainingImages, maxShift, itersPerImage, noise);
    const auto obj = CreateObjectiveFunction(trainingImagePairs);
    std::vector<f64> parameters(OptimizedParameterCount);

    // default
    parameters[BandpassTypeParameter] = static_cast<i32>(mBandpassType);
    parameters[BandpassLParameter] = mBandpassL;
    parameters[BandpassHParameter] = mBandpassH;
    parameters[InterpolationTypeParameter] = static_cast<i32>(mInterpolationType);
    parameters[WindowTypeParameter] = static_cast<i32>(mWindowType);
    parameters[UpsampleCoeffParameter] = mUpsampleCoeff;
    parameters[L1ratioParameter] = mL1ratio;

    noiseStdevs[i] = noise;
    accuracy[i] = obj(parameters);
    progress++;
  }

  Plot1D::Set("NoiseAccuracyDependence");
  Plot1D::SetXlabel("Noise stdev");
  Plot1D::SetYlabel("Average pixel error");
  Plot1D::Plot("NoiseAccuracyDependence", noiseStdevs, accuracy);
}

template <typename Float>
void IterativePhaseCorrelation<Float>::PlotNoiseOptimalBPHDependence(const std::string& trainingImagesDirectory, f64 maxShift, f64 noiseStdev, i32 itersPerImage, i32 iters) const
{
  LOG_FUNCTION("PlotNoiseOptimalBPHDependence");

  if (noiseStdev <= 0.0f)
  {
    LOG_ERROR("Please set some non-zero positive max noise stdev");
    return;
  }

  std::vector<f64> noiseStdevs(iters);
  std::vector<f64> optimalBPHs(iters);
  const f64 xmin = 0;
  const f64 xmax = noiseStdev;
  i32 progress = 0;

  for (i32 i = 0; i < iters; ++i)
  {
    LOG_INFO("Calculating noise stdev optimal BPH dependence ({:.1f}%)", static_cast<f64>(progress) / (iters - 1) * 100);
    f64 noise = xmin + static_cast<f64>(i) / (iters - 1) * (xmax - xmin);

    IterativePhaseCorrelation ipc = *this; // copy this
    ipc.Optimize(trainingImagesDirectory, trainingImagesDirectory, 2.0f, noise, itersPerImage, 0.0f, OptimizedParameterCount * 2);

    noiseStdevs[i] = noise;
    optimalBPHs[i] = ipc.GetBandpassH();
    progress++;
  }

  Plot1D::Set("NoiseOptimalBPHDependence");
  Plot1D::SetXlabel("Noise stdev");
  Plot1D::SetYlabel("Optimal BPH");
  Plot1D::Plot("NoiseOptimalBPHDependence", noiseStdevs, optimalBPHs);
}

template <typename Float>
std::string IterativePhaseCorrelation<Float>::BandpassType2String(BandpassType type)
{
  switch (type)
  {
  case BandpassType::Rectangular:
    return "Rect";
  case BandpassType::Gaussian:
    return "Gauss";
  case BandpassType::None:
    return "None";
  default:
    return "Unknown";
  }
}

template <typename Float>
std::string IterativePhaseCorrelation<Float>::WindowType2String(WindowType type)
{
  switch (type)
  {
  case WindowType::None:
    return "None";
  case WindowType::Hann:
    return "Hann";
  default:
    return "Unknown";
  }
}

template <typename Float>
std::string IterativePhaseCorrelation<Float>::InterpolationType2String(InterpolationType type)
{
  switch (type)
  {
  case InterpolationType::NearestNeighbor:
    return "NN";
  case InterpolationType::Linear:
    return "Linear";
  case InterpolationType::Cubic:
    return "Cubic";
  default:
    return "Unknown";
  }
}

template <typename Float>
void IterativePhaseCorrelation<Float>::ShowDebugStuff() const
try
{
  LOG_FUNCTION("ShowDebugStuff()");

  constexpr bool DebugMode = true;
  constexpr bool addNoise = false;
  constexpr bool debugShift = true;
  constexpr bool debugGradualShift = false;
  constexpr bool debugWindow = false;
  constexpr bool debugBandpass = false;
  constexpr bool debugBandpassRinging = false;

  if constexpr (debugShift)
  {
    cv::Mat image = LoadUnitFloatImage<Float>("../data/AIA/171A.png");
    cv::normalize(image, image, 0, 1, cv::NORM_MINMAX);
    const f64 shiftmax = 10.;
    cv::Point2d shift(RandRange(-1, 1) * shiftmax, RandRange(-1, 1) * shiftmax);
    cv::Point2i point(std::clamp(RandU() * image.cols, static_cast<f64>(mCols), static_cast<f64>(image.cols - mCols)),
        std::clamp(RandU() * image.rows, static_cast<f64>(mRows), static_cast<f64>(image.rows - mRows)));
    cv::Mat Tmat = (cv::Mat_<f64>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
    cv::Mat imageShifted;
    warpAffine(image, imageShifted, Tmat, image.size());
    cv::Mat image1 = RoiCrop(image, point.x, point.y, mCols, mRows);
    cv::Mat image2 = RoiCrop(imageShifted, point.x, point.y, mCols, mRows);
    SetDebugTrueShift(shift);

    if constexpr (addNoise)
    {
      const f64 noiseStdev = 0.01;
      AddNoise<Float>(image1, noiseStdev);
      AddNoise<Float>(image2, noiseStdev);
    }

    auto ipcshift = Calculate<DebugMode>(image1, image2);
    LOG_INFO("Artificial shift = {} / Estimated shift = {} / Error = {}", shift, ipcshift, ipcshift - shift);
  }

  if constexpr (debugGradualShift)
  {
    SetDebugDirectory("../data/peakshift/new");
    const cv::Mat image1 = LoadUnitFloatImage<Float>("../data/AIA/171A.png");
    const cv::Mat crop1 = RoiCropMid(image1, mCols, mRows);
    cv::Mat image2 = image1.clone();
    cv::Mat crop2;
    const i32 iters = 51;
    const f64 totalshift = 2.;
    const f64 noiseStdev = 0.01;
    cv::Mat noise = GetNoise(crop2.size(), noiseStdev);

    if constexpr (addNoise)
      AddNoise<Float>(crop1, noiseStdev);

    for (i32 i = 0; i < iters; i++)
    {
      SetDebugIndex(i);
      const cv::Point2d shift(totalshift * i / (iters - 1), totalshift * i / (iters - 1));
      const cv::Mat Tmat = (cv::Mat_<f64>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
      warpAffine(image1, image2, Tmat, image2.size());
      crop2 = RoiCropMid(image2, mCols, mRows);
      if (addNoise)
        crop2 += noise;
      SetDebugTrueShift(shift);
      const auto ipcshift = Calculate<DebugMode>(crop1, crop2);
      LOG_INFO("Artificial shift = {} / Estimated shift = {} / Error = {}", shift, ipcshift, ipcshift - shift);
    }
  }

  if constexpr (debugWindow)
  {
    cv::Mat img = RoiCrop(LoadUnitFloatImage<Float>("../data/test.png"), 2048, 2048, mCols, mRows);
    cv::Mat w, imgw;
    cv::createHanningWindow(w, img.size(), GetMatType<Float>());
    cv::multiply(img, w, imgw);
    cv::Mat w0 = w.clone();
    cv::Mat r0 = cv::Mat::ones(w.size(), GetMatType<Float>());
    cv::copyMakeBorder(w0, w0, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, cv::BORDER_CONSTANT, cv::Scalar::all(0));
    cv::copyMakeBorder(r0, r0, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, cv::BORDER_CONSTANT, cv::Scalar::all(0));

    // Plot1D::Plot(GetIota(w0.cols, 1), {GetMidRow(r0), GetMidRow(w0)}, "1DWindows", "x", "window", {"cv::Rect", "Hann"}, Plot::pens, mDebugDirectory + "/1DWindows.png");
    // Plot1D::Plot(GetIota(w0.cols, 1), {GetMidRow(Fourier::fftlogmagn(r0)), GetMidRow(Fourier::fftlogmagn(w0))}, "1DWindowsDFT", "fx", "log DFT", {"cv::Rect", "Hann"}, Plot::pens, mDebugDirectory
    // +
    // "/1DWindowsDFT.png");

    // // Plot2D::SetSavePath("IPCdebug2D", mDebugDirectory + "/2DImage.png");
    Plot2D::Plot("IPCdebug2D", img);

    // // Plot2D::SetSavePath("IPCdebug2D", mDebugDirectory + "/2DImageWindow.png");
    Plot2D::Plot("IPCdebug2D", imgw);

    // Plot2D::Plot(Fourier::fftlogmagn(r0), "2DWindowDFTR", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DWindowDFTR.png");
    // Plot2D::Plot(Fourier::fftlogmagn(w0), "2DWindowDFTH", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DWindowDFTH.png");
    // Plot2D::Plot(w, "2DWindow", "x", "y", "window", 0, 1, 0, 1, 0, mDebugDirectory + "/2DWindow.png");
    // Plot2D::Plot(Fourier::fftlogmagn(img), "2DImageDFT", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DImageDFT.png");
    // Plot2D::Plot(Fourier::fftlogmagn(imgw), "2DImageWindowDFT", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DImageWindowDFT.png");
  }

  if constexpr (debugBandpass)
  {
    cv::Mat bpR = cv::Mat(mRows, mCols, GetMatType<Float>());
    cv::Mat bpG = cv::Mat(mRows, mCols, GetMatType<Float>());
    for (i32 r = 0; r < mRows; ++r)
    {
      for (i32 c = 0; c < mCols; ++c)
      {
        bpR.at<Float>(r, c) = BandpassREquation(r, c);

        if (mBandpassL <= 0 and mBandpassH < 1)
          bpG.at<Float>(r, c) = LowpassEquation(r, c);
        else if (mBandpassL > 0 and mBandpassH >= 1)
          bpG.at<Float>(r, c) = HighpassEquation(r, c);
        else if (mBandpassL > 0 and mBandpassH < 1)
          bpG.at<Float>(r, c) = BandpassGEquation(r, c);
      }
    }

    if (mBandpassL > 0 and mBandpassH < 1)
      cv::normalize(bpG, bpG, 0.0, 1.0, cv::NORM_MINMAX);

    cv::Mat bpR0, bpG0;
    cv::copyMakeBorder(bpR, bpR0, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, cv::BORDER_CONSTANT, cv::Scalar::all(0));
    cv::copyMakeBorder(bpG, bpG0, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, cv::BORDER_CONSTANT, cv::Scalar::all(0));

    // Plot1D::Plot(GetIota(bpR0.cols, 1), {GetMidRow(bpR0), GetMidRow(bpG0)}, "b0", "x", "filter", {"cv::Rect", "Gauss"}, Plot::pens, mDebugDirectory + "/1DBandpass.png");
    // Plot1D::Plot(GetIota(bpR0.cols, 1), {GetMidRow(Fourier::ifftlogmagn(bpR0)), GetMidRow(Fourier::ifftlogmagn(bpG0))}, "b1", "fx", "log IDFT", {"cv::Rect", "Gauss"}, Plot::pens, mDebugDirectory
    // +
    // "/1DBandpassIDFT.png"); Plot2D::Plot(bpR, "b2", "x", "y", "filter", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassR.png"); Plot2D::Plot(bpG, "b3", "x", "y", "filter", 0, 1, 0, 1, 0,
    // mDebugDirectory + "/2DBandpassG.png"); Plot2D::Plot(Fourier::ifftlogmagn(bpR0, 10), "b4", "fx", "fy", "log IDFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassRIDFT.png");
    // Plot2D::Plot(Fourier::ifftlogmagn(bpG0, 10), "b5", "fx", "fy", "log IDFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassGIDFT.png");
  }

  if constexpr (debugBandpassRinging)
  {
    cv::Mat img = RoiCrop(LoadUnitFloatImage<Float>("../data/test.png"), 4098 / 2, 4098 / 2, mCols, mRows);
    cv::Mat fftR = Fourier::fft(img);
    cv::Mat fftG = Fourier::fft(img);
    cv::Mat filterR = cv::Mat(img.size(), GetMatType<Float>());
    cv::Mat filterG = cv::Mat(img.size(), GetMatType<Float>());

    for (i32 r = 0; r < mRows; ++r)
    {
      for (i32 c = 0; c < mCols; ++c)
      {
        filterR.at<Float>(r, c) = BandpassREquation(r, c);

        if (mBandpassL <= 0 and mBandpassH < 1)
          filterG.at<Float>(r, c) = LowpassEquation(r, c);
        else if (mBandpassL > 0 and mBandpassH >= 1)
          filterG.at<Float>(r, c) = HighpassEquation(r, c);
        else if (mBandpassL > 0 and mBandpassH < 1)
          filterG.at<Float>(r, c) = BandpassGEquation(r, c);
      }
    }

    Fourier::ifftshift(filterR);
    Fourier::ifftshift(filterG);

    cv::Mat filterRc = Fourier::dupchansc(filterR);
    cv::Mat filterGc = Fourier::dupchansc(filterG);

    cv::multiply(fftR, filterRc, fftR);
    cv::multiply(fftG, filterGc, fftG);

    cv::Mat imgfR = Fourier::ifft(fftR);
    cv::Mat imgfG = Fourier::ifft(fftG);

    cv::normalize(imgfR, imgfR, 0.0, 1.0, cv::NORM_MINMAX);
    cv::normalize(imgfG, imgfG, 0.0, 1.0, cv::NORM_MINMAX);

    // // Plot2D::SetSavePath("IPCdebug2D", mDebugDirectory + "/2DBandpassImageR.png");
    Plot2D::Plot("IPCdebug2D", imgfR);

    // // Plot2D::SetSavePath("IPCdebug2D", mDebugDirectory + "/2DBandpassImageG.png");
    Plot2D::Plot("IPCdebug2D", imgfG);
  }

  LOG_INFO("IPC debug stuff shown");
}
catch (const std::exception& e)
{
  LOG_ERROR("ShowDebugStuff() error: {}", e.what());
}

template <typename Float>
void IterativePhaseCorrelation<Float>::DebugInputImages(const cv::Mat& image1, const cv::Mat& image2) const
{
  PyPlot::Plot(fmt::format("{} I1", mDebugName), {.z = image1, .cmap = "gray"});
  PyPlot::Plot(fmt::format("{} I2", mDebugName), {.z = image2, .cmap = "gray"});
}

template <typename Float>
void IterativePhaseCorrelation<Float>::DebugFourierTransforms(const cv::Mat& dft1, const cv::Mat& dft2) const
{
  auto plot1 = dft1.clone();
  Fourier::fftshift(plot1);
  PyPlot::Plot(fmt::format("{} DFT1lm", mDebugName), {.z = Fourier::logmagn(plot1)});
  PyPlot::Plot(fmt::format("{} DFT1p", mDebugName), {.z = Fourier::phase(plot1)});

  auto plot2 = dft2.clone();
  Fourier::fftshift(plot2);
  PyPlot::Plot(fmt::format("{} DFT2lm", mDebugName), {.z = Fourier::logmagn(plot2)});
  PyPlot::Plot(fmt::format("{} DFT2p", mDebugName), {.z = Fourier::phase(plot2)});
}

template <typename Float>
void IterativePhaseCorrelation<Float>::DebugCrossPowerSpectrum(const cv::Mat& crosspower) const
{
  PyPlot::Plot(fmt::format("{} CP cv::magnitude", mDebugName), {.z = Fourier::fftshift(Fourier::magn(crosspower))});
  PyPlot::Plot(fmt::format("{} CP cv::magnitude 1D", mDebugName), {.y = GetMidRow<Float>(Fourier::fftshift(Fourier::magn(crosspower)))});
  PyPlot::Plot(fmt::format("{} CP cv::phase", mDebugName), {.z = Fourier::fftshift(Fourier::phase(crosspower))});
  PyPlot::Plot(fmt::format("{} CP sawtooth", mDebugName),
      {.ys = {GetRow<Float>(Fourier::fftshift(Fourier::phase(crosspower)), 0.6 * crosspower.rows), GetCol<Float>(Fourier::fftshift(Fourier::phase(crosspower)), 0.6 * crosspower.cols)},
          .label_ys = {"x", "y"}});
}

template <typename Float>
void IterativePhaseCorrelation<Float>::DebugL3(const cv::Mat& L3) const
{
  PyPlot::Plot(fmt::format("{} L3", mDebugName), {.z = L3});
}

template <typename Float>
void IterativePhaseCorrelation<Float>::DebugL2(const cv::Mat& L2) const
{
  auto plot = L2.clone();
  cv::resize(plot, plot, plot.size() * mUpsampleCoeff, 0, 0, cv::INTER_NEAREST);
  PyPlot::Plot(fmt::format("{} L2", mDebugName), {.z = plot});
}

template <typename Float>
void IterativePhaseCorrelation<Float>::DebugL2U(const cv::Mat& L2, const cv::Mat& L2U) const
{
  PyPlot::Plot(fmt::format("{} L2U", mDebugName), {.z = L2U});

  if (0)
  {
    cv::Mat nearest, linear, cubic;
    cv::resize(L2, nearest, L2.size() * mUpsampleCoeff, 0, 0, cv::INTER_NEAREST);
    cv::resize(L2, linear, L2.size() * mUpsampleCoeff, 0, 0, cv::INTER_LINEAR);
    cv::resize(L2, cubic, L2.size() * mUpsampleCoeff, 0, 0, cv::INTER_CUBIC);

    PyPlot::Plot("IPCL2UN", {.z = nearest});
    PyPlot::Plot("IPCL2UL", {.z = linear});
    PyPlot::Plot("IPCL2UC", {.z = cubic});
  }
}

template <typename Float>
void IterativePhaseCorrelation<Float>::DebugL1B(const cv::Mat& L2U, i32 L1size, const cv::Point2d& L3shift) const
{
  cv::Mat mat = CalculateL1(L2U, cv::Point(L2U.cols / 2, L2U.rows / 2), L1size).clone();
  cv::normalize(mat, mat, 0, 1, cv::NORM_MINMAX); // for black crosshairs + cross
  mat = mat.mul(Kirkl<Float>(mat.rows));
  DrawCrosshairs(mat);
  DrawCross(mat, cv::Point2d(mat.cols / 2, mat.rows / 2) + mUpsampleCoeff * (mDebugTrueShift - L3shift));
  cv::resize(mat, mat, cv::Size(mUpsampleCoeff * mL2size, mUpsampleCoeff * mL2size), 0, 0, cv::INTER_NEAREST);
  PyPlot::Plot(fmt::format("{} L1B", mDebugName), {.z = mat, .save = not mDebugDirectory.empty() ? fmt::format("{}/L1B_{}.png", mDebugDirectory, mDebugIndex) : ""});
}

template <typename Float>
void IterativePhaseCorrelation<Float>::DebugL1A(const cv::Mat& L1, const cv::Point2d& L3shift, const cv::Point2d& L2Ushift, bool last) const
{
  cv::Mat mat = L1.clone();
  cv::normalize(mat, mat, 0, 1, cv::NORM_MINMAX); // for black crosshairs + cross
  mat = mat.mul(Kirkl<Float>(mat.rows));
  DrawCrosshairs(mat);
  DrawCross(mat, cv::Point2d(mat.cols / 2, mat.rows / 2) + mUpsampleCoeff * (mDebugTrueShift - L3shift) - L2Ushift);
  cv::resize(mat, mat, cv::Size(mUpsampleCoeff * mL2size, mUpsampleCoeff * mL2size), 0, 0, cv::INTER_NEAREST);
  PyPlot::Plot(fmt::format("{} L1A", mDebugName), {.z = mat, .save = not mDebugDirectory.empty() and last ? fmt::format("{}/L1A_{}.png", mDebugDirectory, mDebugIndex) : ""});
}

template <typename Float>
cv::Mat IterativePhaseCorrelation<Float>::ColorComposition(const cv::Mat& img1, const cv::Mat& img2)
{
  PROFILE_FUNCTION;
  const cv::Vec<Float, 3> img1clr = {1, 0.5, 0};
  const cv::Vec<Float, 3> img2clr = {0, 0.5, 1};

  const f64 gamma1 = 1.0;
  const f64 gamma2 = 1.0;

  cv::Mat img1c = cv::Mat(img1.size(), GetMatType<Float>(3));
  cv::Mat img2c = cv::Mat(img2.size(), GetMatType<Float>(3));

  for (i32 row = 0; row < img1.rows; ++row)
  {
    auto img1p = img1.ptr<Float>(row);
    auto img2p = img2.ptr<Float>(row);
    auto img1cp = img1c.ptr<cv::Vec<Float, 3>>(row);
    auto img2cp = img2c.ptr<cv::Vec<Float, 3>>(row);

    for (i32 col = 0; col < img1.cols; ++col)
    {
      img1cp[col][0] = pow(img1clr[2] * img1p[col], 1. / gamma1);
      img1cp[col][1] = pow(img1clr[1] * img1p[col], 1. / gamma1);
      img1cp[col][2] = pow(img1clr[0] * img1p[col], 1. / gamma1);

      img2cp[col][0] = pow(img2clr[2] * img2p[col], 1. / gamma2);
      img2cp[col][1] = pow(img2clr[1] * img2p[col], 1. / gamma2);
      img2cp[col][2] = pow(img2clr[0] * img2p[col], 1. / gamma2);
    }
  }

  if (gamma1 != 1 or gamma2 != 1)
  {
    cv::normalize(img1c, img1c, 0, 1, cv::NORM_MINMAX);
    cv::normalize(img2c, img2c, 0, 1, cv::NORM_MINMAX);
  }

  return (img1c + img2c) / 2;
}

template <typename Float>
std::vector<cv::Mat> IterativePhaseCorrelation<Float>::LoadImages(const std::string& imagesDirectory)
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

    if (not(path.find(".png") != std::string::npos or path.find(".PNG") != std::string::npos or path.find(".jpg") != std::string::npos or path.find(".JPG") != std::string::npos ||
            path.find(".jpeg") != std::string::npos or path.find(".JPEG") != std::string::npos or path.find(".fits") != std::string::npos or path.find(".FITS") != std::string::npos))
    {
      LOG_DEBUG("Directory contains a non-image file {}", path);
      continue;
    }

    // crop the input image - good for solar images, omits the black borders
    static constexpr f64 cropFocusRatio = 0.5;
    auto image = LoadUnitFloatImage<Float>(path);
    image = RoiCropMid(image, cropFocusRatio * image.cols, cropFocusRatio * image.rows);
    images.push_back(image);
    LOG_DEBUG("Loaded image {}", path);
  }
  return images;
}

template <typename Float>
std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>> IterativePhaseCorrelation<Float>::CreateImagePairs(const std::vector<cv::Mat>& images, f64 maxShift, i32 itersPerImage, f64 noiseStdev) const
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("CreateImagePairs");

  if (maxShift <= 0)
    throw std::runtime_error(fmt::format("Invalid max shift ({})", maxShift));
  if (itersPerImage < 1)
    throw std::runtime_error(fmt::format("Invalid iters per image ({})", itersPerImage));
  if (noiseStdev < 0)
    throw std::runtime_error(fmt::format("Invalid noise stdev ({})", noiseStdev));

  if (const auto badimage = std::find_if(images.begin(), images.end(), [&](const auto& image) { return image.rows < mRows + maxShift or image.cols < mCols + maxShift; }); badimage != images.end())
    throw std::runtime_error(fmt::format("Could not optimize IPC parameters - input image is too small for specified IPC window size & max shift ratio ([{},{}] < [{},{}])", badimage->rows,
        badimage->cols, mRows + maxShift, mCols + maxShift));

  std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>> imagePairs;
  imagePairs.reserve(images.size() * itersPerImage);

  for (const auto& image : images)
  {
    for (i32 i = 0; i < itersPerImage; ++i)
    {
      // random shift from a random point
      cv::Point2d shift(RandRange(-1, 1) * maxShift, RandRange(-1, 1) * maxShift);
      cv::Point2i point(std::clamp(RandU() * image.cols, static_cast<f64>(mCols), static_cast<f64>(image.cols - mCols)),
          std::clamp(RandU() * image.rows, static_cast<f64>(mRows), static_cast<f64>(image.rows - mRows)));
      cv::Mat Tmat = (cv::Mat_<f64>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
      cv::Mat imageShifted;
      warpAffine(image, imageShifted, Tmat, image.size());
      cv::Mat image1 = RoiCrop(image, point.x, point.y, mCols, mRows);
      cv::Mat image2 = RoiCrop(imageShifted, point.x, point.y, mCols, mRows);

      ConvertToUnitFloat<false>(image1);
      ConvertToUnitFloat<false>(image2);

      AddNoise<Float>(image1, noiseStdev);
      AddNoise<Float>(image2, noiseStdev);

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

template <typename Float>
IterativePhaseCorrelation<Float> IterativePhaseCorrelation<Float>::CreateIPCFromParams(const std::vector<f64>& params) const
{
  PROFILE_FUNCTION;
  IterativePhaseCorrelation ipc(mRows, mCols);
  ipc.SetBandpassType(static_cast<BandpassType>((i32)params[BandpassTypeParameter]));
  ipc.SetBandpassParameters(params[BandpassLParameter], params[BandpassHParameter]);
  ipc.SetInterpolationType(static_cast<InterpolationType>((i32)params[InterpolationTypeParameter]));
  ipc.SetWindowType(static_cast<WindowType>((i32)params[WindowTypeParameter]));
  ipc.SetUpsampleCoeff(params[UpsampleCoeffParameter]);
  ipc.SetL1ratio(params[L1ratioParameter]);
  return ipc;
}

template <typename Float>
std::function<f64(const std::vector<f64>&)> IterativePhaseCorrelation<Float>::CreateObjectiveFunction(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs) const
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("CreateObjectiveFunction");
  return [&](const std::vector<f64>& params) {
    const auto ipc = CreateIPCFromParams(params);

    if (std::floor(ipc.GetL2size() * ipc.GetUpsampleCoeff() * ipc.GetL1ratio()) < 3)
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

template <typename Float>
std::function<f64(const std::vector<f64>&)> IterativePhaseCorrelation<Float>::CreateObjectiveFunction(const std::function<f64(const IterativePhaseCorrelation&)>& obj) const
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("CreateObjectiveFunction");
  return [&](const std::vector<f64>& params) {
    const auto ipc = CreateIPCFromParams(params);

    if (std::floor(ipc.GetL2size() * ipc.GetUpsampleCoeff() * ipc.GetL1ratio()) < 3)
      return std::numeric_limits<f64>::max();

    return obj(ipc);
  };
}

template <typename Float>
std::vector<f64> IterativePhaseCorrelation<Float>::CalculateOptimalParameters(
    const std::function<f64(const std::vector<f64>&)>& obj, const std::function<f64(const std::vector<f64>&)>& valid, i32 populationSize)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("CalculateOptimalParameters");

  if (populationSize < 4)
    throw std::runtime_error(fmt::format("Invalid population size ({})", populationSize));

  Evolution evo(OptimizedParameterCount);
  evo.mNP = populationSize;
  evo.mMutStrat = Evolution::RAND1;
  evo.SetParameterNames({"BP", "BPL", "BPH", "INT", "WIN", "UC", "L1R"});
  evo.mLB = {0, -0.5, 0.0, 0, 0, 3, 0.1};
  evo.mUB = {static_cast<f64>(BandpassType::BandpassTypeCount) - 1e-8, 0.5, 2.0, static_cast<f64>(InterpolationType::InterpolationTypeCount) - 1e-8,
      static_cast<f64>(WindowType::WindowTypeCount) - 1e-8, 51, 0.8};
  evo.SetPlotOutput(true);
  evo.SetConsoleOutput(true);
  evo.SetParameterValueToNameFunction("BP", [](f64 val) { return BandpassType2String(static_cast<BandpassType>((i32)val)); });
  evo.SetParameterValueToNameFunction("BPL", [](f64 val) { return fmt::format("{:.2f}", val); });
  evo.SetParameterValueToNameFunction("BPH", [](f64 val) { return fmt::format("{:.2f}", val); });
  evo.SetParameterValueToNameFunction("INT", [](f64 val) { return InterpolationType2String(static_cast<InterpolationType>((i32)val)); });
  evo.SetParameterValueToNameFunction("WIN", [](f64 val) { return WindowType2String(static_cast<WindowType>((i32)val)); });
  evo.SetParameterValueToNameFunction("UC", [](f64 val) { return fmt::format("{}", static_cast<i32>(val)); });
  evo.SetParameterValueToNameFunction("L1R", [](f64 val) { return fmt::format("{:.2f}", val); });
  return evo.Optimize(obj, valid).optimum;
}

template <typename Float>
void IterativePhaseCorrelation<Float>::ApplyOptimalParameters(const std::vector<f64>& optimalParameters)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("ApplyOptimalParameters");

  if (optimalParameters.size() < OptimizedParameterCount)
    throw std::runtime_error("Cannot apply optimal parameters - wrong parameter count");

  const auto thisBefore = *this;

  SetBandpassType(static_cast<BandpassType>((i32)optimalParameters[BandpassTypeParameter]));
  SetBandpassParameters(optimalParameters[BandpassLParameter], optimalParameters[BandpassHParameter]);
  SetInterpolationType(static_cast<InterpolationType>((i32)optimalParameters[InterpolationTypeParameter]));
  SetWindowType(static_cast<WindowType>((i32)optimalParameters[WindowTypeParameter]));
  SetUpsampleCoeff(optimalParameters[UpsampleCoeffParameter]);
  SetL1ratio(optimalParameters[L1ratioParameter]);

  LOG_SUCCESS("Final IPC BandpassType: {} -> {}", BandpassType2String(thisBefore.GetBandpassType()), BandpassType2String(GetBandpassType()));
  if (GetBandpassType() != BandpassType::None)
  {
    LOG_SUCCESS("Final IPC BandpassL: {:.2f} -> {:.2f}", thisBefore.GetBandpassL(), GetBandpassL());
    LOG_SUCCESS("Final IPC BandpassH: {:.2f} -> {:.2f}", thisBefore.GetBandpassH(), GetBandpassH());
  }
  LOG_SUCCESS("Final IPC InterpolationType: {} -> {}", InterpolationType2String(thisBefore.GetInterpolationType()), InterpolationType2String(GetInterpolationType()));
  LOG_SUCCESS("Final IPC WindowType: {} -> {}", WindowType2String(thisBefore.GetWindowType()), WindowType2String(GetWindowType()));
  LOG_SUCCESS("Final IPC UpsampleCoeff: {} -> {}", thisBefore.GetUpsampleCoeff(), GetUpsampleCoeff());
  LOG_SUCCESS("Final IPC L1ratio: {:.2f} -> {:.2f}", thisBefore.GetL1ratio(), GetL1ratio());
}

template <typename Float>
void IterativePhaseCorrelation<Float>::ShowOptimizationPlots(const std::vector<cv::Point2d>& shiftsReference, const std::vector<cv::Point2d>& shiftsPixel, const std::vector<cv::Point2d>& shiftsNonit,
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

template <typename Float>
std::vector<cv::Point2d> IterativePhaseCorrelation<Float>::GetShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs) const
{
  PROFILE_FUNCTION;
  std::vector<cv::Point2d> out;
  out.reserve(imagePairs.size());

  for (const auto& [image1, image2, referenceShift] : imagePairs)
    out.push_back(Calculate(image1, image2));

  return out;
}

template <typename Float>
std::vector<cv::Point2d> IterativePhaseCorrelation<Float>::GetNonIterativeShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs) const
{
  PROFILE_FUNCTION;
  std::vector<cv::Point2d> out;
  out.reserve(imagePairs.size());

  for (const auto& [image1, image2, referenceShift] : imagePairs)
    out.push_back(Calculate<false, false, AccuracyType::Subpixel>(image1, image2));

  return out;
}

template <typename Float>
std::vector<cv::Point2d> IterativePhaseCorrelation<Float>::GetPixelShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs) const
{
  PROFILE_FUNCTION;
  std::vector<cv::Point2d> out;
  out.reserve(imagePairs.size());

  for (const auto& [image1, image2, referenceShift] : imagePairs)
    out.push_back(Calculate<false, false, AccuracyType::Pixel>(image1, image2));

  return out;
}

template <typename Float>
std::vector<cv::Point2d> IterativePhaseCorrelation<Float>::GetReferenceShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs)
{
  PROFILE_FUNCTION;
  std::vector<cv::Point2d> out;
  out.reserve(imagePairs.size());

  for (const auto& [image1, image2, referenceShift] : imagePairs)
    out.push_back(referenceShift);

  return out;
}

template <typename Float>
f64 IterativePhaseCorrelation<Float>::GetAverageAccuracy(const std::vector<cv::Point2d>& shiftsReference, const std::vector<cv::Point2d>& shifts)
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

template <typename Float>
void IterativePhaseCorrelation<Float>::ShowRandomImagePair(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs)
{
  PROFILE_FUNCTION;
  LOG_FUNCTION("ShowRandomImagePair");

  const auto& [img1, img2, shift] = imagePairs[static_cast<usize>(RandU() * imagePairs.size())];
  cv::Mat concat;
  cv::hconcat(img1, img2, concat);
  Plot2D::Set("Random image pair");
  Plot2D::SetColorMapType(QCPColorGradient::gpGrayscale);
  Plot2D::Plot("Random image pair", concat);
}
