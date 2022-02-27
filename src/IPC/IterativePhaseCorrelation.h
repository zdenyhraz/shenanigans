#pragma once
#include "Fourier/Fourier.h"
#include "Optimization/Evolution.h"
#include "UtilsCV/Vectmat.h"

template <typename Float = f64>
class IterativePhaseCorrelation
{
public:
  enum class BandpassType : u8
  {
    Rectangular,
    Gaussian,
    None,
    BandpassTypeCount // last
  };

  enum class WindowType : u8
  {
    None,
    Hann,
    WindowTypeCount // last
  };

  enum class InterpolationType : u8
  {
    NearestNeighbor,
    Linear,
    Cubic,
    InterpolationTypeCount // last
  };

  enum class AccuracyType : u8
  {
    Pixel,
    Subpixel,
    SubpixelIterative
  };

  enum OptimizedParameters
  {
    BandpassTypeParameter,
    BandpassLParameter,
    BandpassHParameter,
    InterpolationTypeParameter,
    WindowTypeParameter,
    UpsampleCoeffParameter,
    L1ratioParameter,
    OptimizedParameterCount, // last
  };

  explicit IterativePhaseCorrelation(i32 rows, i32 cols = -1, f64 bandpassL = 0, f64 bandpassH = 1) { Initialize(rows, cols, bandpassL, bandpassH); }
  explicit IterativePhaseCorrelation(const cv::Size& size, f64 bandpassL = 0, f64 bandpassH = 1) { Initialize(size.height, size.width, bandpassL, bandpassH); }
  explicit IterativePhaseCorrelation(const cv::Mat& img, f64 bandpassL = 0, f64 bandpassH = 1) { Initialize(img.rows, img.cols, bandpassL, bandpassH); }

  void Initialize(i32 rows, i32 cols, f64 bandpassL, f64 bandpassH)
  {
    PROFILE_FUNCTION;
    SetSize(rows, cols);
    SetBandpassParameters(bandpassL, bandpassH);
    UpdateL1circle();
  }
  void SetSize(i32 rows, i32 cols = -1)
  {
    PROFILE_FUNCTION;
    mRows = rows;
    mCols = cols > 0 ? cols : rows;

    if (mWindow.rows != mRows or mWindow.cols != mCols)
      UpdateWindow();

    if (mBandpass.rows != mRows or mBandpass.cols != mCols)
      UpdateBandpass();
  }
  void SetSize(const cv::Size& size) { SetSize(size.height, size.width); }
  void SetBandpassParameters(f64 bandpassL, f64 bandpassH)
  {
    mBandpassL = std::clamp(bandpassL, 0., std::numeric_limits<f64>::max());
    mBandpassH = std::clamp(bandpassH, 0., std::numeric_limits<f64>::max());
    UpdateBandpass();
  }
  void SetBandpassType(BandpassType type)
  {
    mBandpassType = type;
    UpdateBandpass();
  }
  void SetL2size(i32 L2size)
  {
    mL2size = L2size % 2 ? L2size : L2size + 1;
    UpdateL1circle();
  }
  void SetL1ratio(f64 L1ratio)
  {
    mL1ratio = L1ratio;
    UpdateL1circle();
  }
  void SetUpsampleCoeff(i32 upsampleCoeff)
  {
    mUpsampleCoeff = upsampleCoeff % 2 ? upsampleCoeff : upsampleCoeff + 1;
    UpdateL1circle();
  }
  void SetMaxIterations(i32 maxIterations) { mMaxIterations = maxIterations; }
  void SetInterpolationType(InterpolationType interpolationType) { mInterpolationType = interpolationType; }
  void SetWindowType(WindowType type)
  {
    mWindowType = type;
    UpdateWindow();
  }
  void SetDebugDirectory(const std::string& dir) const
  {
    mDebugDirectory = dir;
    if (not std::filesystem::exists(dir))
      std::filesystem::create_directory(dir);
  }
  void SetDebugName(const std::string& name) const { mDebugName = name; }
  void SetDebugIndex(i32 index) const { mDebugIndex = index; }
  void SetDebugTrueShift(const cv::Point2d& shift) const { mDebugTrueShift = shift; }

  i32 GetRows() const { return mRows; }
  i32 GetCols() const { return mCols; }
  i32 GetSize() const { return mRows; }
  f64 GetBandpassL() const { return mBandpassL; }
  f64 GetBandpassH() const { return mBandpassH; }
  i32 GetL2size() const { return mL2size; }
  f64 GetL1ratio() const { return mL1ratio; }
  i32 GetUpsampleCoeff() const { return mUpsampleCoeff; }
  cv::Mat GetWindow() const { return mWindow; }
  cv::Mat GetBandpass() const { return mBandpass; }
  BandpassType GetBandpassType() const { return mBandpassType; }
  WindowType GetWindowType() const { return mWindowType; }
  InterpolationType GetInterpolationType() const { return mInterpolationType; }

  template <bool DebugMode = false, bool CrossCorrelation = false, AccuracyType AccuracyT = AccuracyType::SubpixelIterative>
  cv::Point2d Calculate(const cv::Mat& image1, const cv::Mat& image2) const
  {
    PROFILE_FUNCTION;
    return Calculate<DebugMode, CrossCorrelation, AccuracyT>(image1.clone(), image2.clone());
  }

  template <bool DebugMode = false, bool CrossCorrelation = false, AccuracyType AccuracyT = AccuracyType::SubpixelIterative>
  cv::Point2d Calculate(cv::Mat&& image1, cv::Mat&& image2) const
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::Calculate");

    if (image1.size() != cv::Size(mCols, mRows))
      [[unlikely]] throw std::invalid_argument(fmt::format("Invalid image size ({} != {})", image1.size(), cv::Size(mCols, mRows)));

    if (image1.size() != image2.size())
      [[unlikely]] throw std::invalid_argument(fmt::format("Image sizes differ ({} != {})", image1.size(), image2.size()));

    if (image1.channels() != 1 or image2.channels() != 1)
      [[unlikely]] throw std::invalid_argument("Multichannel images are not supported");

    ConvertToUnitFloat<DebugMode, CrossCorrelation>(image1);
    ConvertToUnitFloat<DebugMode, CrossCorrelation>(image2);
    if constexpr (DebugMode)
      DebugInputImages(image1, image2);

    // windowing
    ApplyWindow<DebugMode>(image1);
    ApplyWindow<DebugMode>(image2);

    // ffts
    auto dft1 = CalculateFourierTransform<DebugMode>(std::move(image1));
    auto dft2 = CalculateFourierTransform<DebugMode>(std::move(image2));
    if constexpr (DebugMode and 0)
      DebugFourierTransforms(dft1, dft2);

    // cross-power
    auto crosspower = CalculateCrossPowerSpectrum<DebugMode, CrossCorrelation>(std::move(dft1), std::move(dft2));
    if constexpr (DebugMode)
      DebugCrossPowerSpectrum(crosspower);

    // L3
    cv::Mat L3 = CalculateL3<DebugMode>(std::move(crosspower));
    cv::Point2d L3peak = GetPeak<DebugMode>(L3);
    cv::Point2d L3mid(L3.cols / 2, L3.rows / 2);
    if constexpr (DebugMode)
      DebugL3(L3);

    if constexpr (AccuracyT == AccuracyType::Pixel)
      return GetPixelShift(L3peak, L3mid);

    if constexpr (AccuracyT == AccuracyType::Subpixel)
      return GetSubpixelShift<DebugMode>(L3, L3peak, L3mid, 5);

    // reduce the L2size as long as the L2 is out of bounds, return pixel level estimation accuracy if it cannot be reduced anymore
    i32 L2size = mL2size;
    while (IsOutOfBounds(L3peak, L3, L2size))
      if (!ReduceL2size<DebugMode>(L2size))
        return L3peak - L3mid;

    // L2
    cv::Mat L2 = CalculateL2<DebugMode>(L3, L3peak, L2size);
    cv::Point2d L2mid(L2.cols / 2, L2.rows / 2);
    if constexpr (DebugMode)
      DebugL2(L2);

    // L2U
    cv::Mat L2U = CalculateL2U<DebugMode>(L2);
    cv::Point2d L2Umid(L2U.cols / 2, L2U.rows / 2);
    if constexpr (DebugMode)
      DebugL2U(L2, L2U);

    f64 L1ratio = mL1ratio;
    cv::Mat L1circle = mL1circle.clone();
    while (L1ratio > 0)
    {
      PROFILE_SCOPE(IterativeRefinement);
      LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::IterativeRefinement");

      // reset L2U peak position
      cv::Point2d L2Upeak = L2Umid;

      // L1
      cv::Mat L1;
      i32 L1size = GetL1size(L2U.cols, L1ratio);
      cv::Point2d L1mid(L1size / 2, L1size / 2);
      cv::Point2d L1peak;
      if (L1circle.cols != L1size)
        [[unlikely]] L1circle = kirkl<Float>(L1size);
      if constexpr (DebugMode)
        DebugL1B(L2U, L1size, L3peak - L3mid);

      for (i32 iter = 0; iter < mMaxIterations; ++iter)
      {
        PROFILE_SCOPE(IterativeRefinementIteration);
        if (IsOutOfBounds(L2Upeak, L2U, L1size))
          [[unlikely]] break;

        L1 = CalculateL1(L2U, L2Upeak, L1size);
        if constexpr (DebugMode)
          DebugL1A(L1, L3peak - L3mid, L2Upeak - L2Umid);
        L1peak = GetPeakSubpixel<true>(L1, L1circle);
        L2Upeak += cv::Point2d(std::round(L1peak.x - L1mid.x), std::round(L1peak.y - L1mid.y));

        if (AccuracyReached(L1peak, L1mid))
          [[unlikely]]
          {
            if constexpr (DebugMode)
              DebugL1A(L1, L3peak - L3mid, L2Upeak - cv::Point2d(std::round(L1peak.x - L1mid.x), std::round(L1peak.y - L1mid.y)) - L2Umid, true);
            return L3peak - L3mid + (L2Upeak - L2Umid + L1peak - L1mid) / mUpsampleCoeff;
          }
      }

      // maximum iterations reached - reduce L1 size by reducing L1ratio
      ReduceL1ratio<DebugMode>(L1ratio);
    }

    // L1 failed to converge with all L1ratios, return non-iterative subpixel shift
    return GetSubpixelShift<DebugMode>(L3, L3peak, L3mid, L2size);
  }

  cv::Mat Align(const cv::Mat& image1, const cv::Mat& image2) const { return Align(image1.clone(), image2.clone()); }
  cv::Mat Align(cv::Mat&& image1, cv::Mat&& image2) const
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
    cv::Mat img1FTm = cv::Mat::zeros(img1FT.size(), GetMatType<Float>());
    cv::Mat img2FTm = cv::Mat::zeros(img2FT.size(), GetMatType<Float>());
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
      showimg(ColorComposition(image1, image2), "color composition result", 0, 0, 1, 1000);
    }
    return image2;
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("IPC Align error: {}", e.what());
    return cv::Mat();
  }
  std::tuple<cv::Mat, cv::Mat> CalculateFlow(const cv::Mat& image1, const cv::Mat& image2, f64 resolution) const { return CalculateFlow(image1.clone(), image2.clone(), resolution); }
  std::tuple<cv::Mat, cv::Mat> CalculateFlow(cv::Mat&& image1, cv::Mat&& image2, f64 resolution) const
  try
  {
    PROFILE_FUNCTION;
    if (image1.size() != image2.size())
      throw std::runtime_error(fmt::format("Image sizes differ ({} != {})", image1.size(), image2.size()));

    if (mRows > image1.rows or mCols > image1.cols)
      throw std::runtime_error(fmt::format("Images are too small ({} < {})", image1.size(), cv::Size(mCols, mRows)));

    cv::Mat flowX = cv::Mat::zeros(cv::Size(resolution * image1.cols, resolution * image1.rows), GetMatType<Float>());
    cv::Mat flowY = cv::Mat::zeros(cv::Size(resolution * image2.cols, resolution * image2.rows), GetMatType<Float>());
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

        const auto shift = Calculate(roicrop(image1, center.x, center.y, mCols, mRows), roicrop(image2, center.x, center.y, mCols, mRows));
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
  void Optimize(const std::string& trainingImagesDirectory, const std::string& validationImagesDirectory, f64 maxShift = 2.0, f64 noiseStdev = 0.01, i32 itersPerImage = 100, f64 validationRatio = 0.2,
      i32 populationSize = OptimizedParameterCount * 7)
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

  void Optimize(const std::function<f64(const IterativePhaseCorrelation&)>& obj, i32 populationSize = OptimizedParameterCount * 7)
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
  void PlotObjectiveFunctionLandscape(const std::string& trainingImagesDirectory, f64 maxShift, f64 noiseStdev, i32 itersPerImage, i32 iters) const
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

  void PlotImageSizeAccuracyDependence(const std::string& trainingImagesDirectory, f64 maxShift, f64 noiseStdev, i32 itersPerImage, i32 iters)
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

  void PlotUpsampleCoefficientAccuracyDependence(const std::string& trainingImagesDirectory, f64 maxShift, f64 noiseStdev, i32 itersPerImage, i32 iters) const
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

  void PlotNoiseAccuracyDependence(const std::string& trainingImagesDirectory, f64 maxShift, f64 noiseStdev, i32 itersPerImage, i32 iters) const
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

  void PlotNoiseOptimalBPHDependence(const std::string& trainingImagesDirectory, f64 maxShift, f64 noiseStdev, i32 itersPerImage, i32 iters) const
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
  static std::string BandpassType2String(BandpassType type)
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

  static std::string WindowType2String(WindowType type)
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

  static std::string InterpolationType2String(InterpolationType type)
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
  void ShowDebugStuff() const
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
      cv::Point2d shift(rand11() * shiftmax, rand11() * shiftmax);
      cv::Point2i point(clamp(rand01() * image.cols, mCols, image.cols - mCols), clamp(rand01() * image.rows, mRows, image.rows - mRows));
      cv::Mat Tmat = (cv::Mat_<f64>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
      cv::Mat imageShifted;
      warpAffine(image, imageShifted, Tmat, image.size());
      cv::Mat image1 = roicrop(image, point.x, point.y, mCols, mRows);
      cv::Mat image2 = roicrop(imageShifted, point.x, point.y, mCols, mRows);
      SetDebugTrueShift(shift);

      if constexpr (addNoise)
      {
        const f64 noiseStdev = 0.01;
        cv::Mat noise1 = cv::Mat::zeros(image1.rows, image1.cols, GetMatType<Float>());
        cv::Mat noise2 = cv::Mat::zeros(image2.rows, image2.cols, GetMatType<Float>());
        randn(noise1, 0, noiseStdev);
        randn(noise2, 0, noiseStdev);
        image1 += noise1;
        image2 += noise2;
      }

      auto ipcshift = Calculate<DebugMode>(image1, image2);
      LOG_INFO("Artificial shift = {} / Estimated shift = {} / Error = {}", shift, ipcshift, ipcshift - shift);
    }

    if constexpr (debugGradualShift)
    {
      SetDebugDirectory("../data/peakshift/new");
      const cv::Mat image1 = LoadUnitFloatImage<Float>("../data/AIA/171A.png");
      const cv::Mat crop1 = roicropmid(image1, mCols, mRows);
      cv::Mat image2 = image1.clone();
      cv::Mat crop2;
      const i32 iters = 51;
      const f64 totalshift = 2.;
      const f64 noiseStdev = 0.01;
      cv::Mat noise1, noise2;

      if constexpr (addNoise)
      {
        noise1 = cv::Mat::zeros(crop1.rows, crop1.cols, GetMatType<Float>());
        noise2 = cv::Mat::zeros(crop1.rows, crop1.cols, GetMatType<Float>());
        randn(noise1, 0, noiseStdev);
        randn(noise2, 0, noiseStdev);
        crop1 += noise1;
      }
      for (i32 i = 0; i < iters; i++)
      {
        SetDebugIndex(i);
        const cv::Point2d shift(totalshift * i / (iters - 1), totalshift * i / (iters - 1));
        const cv::Mat Tmat = (cv::Mat_<f64>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
        warpAffine(image1, image2, Tmat, image2.size());
        crop2 = roicropmid(image2, mCols, mRows);
        if (addNoise)
          crop2 += noise2;
        SetDebugTrueShift(shift);
        const auto ipcshift = Calculate<DebugMode>(crop1, crop2);
        LOG_INFO("Artificial shift = {} / Estimated shift = {} / Error = {}", shift, ipcshift, ipcshift - shift);
      }
    }

    if constexpr (debugWindow)
    {
      cv::Mat img = roicrop(LoadUnitFloatImage<Float>("../data/test.png"), 2048, 2048, mCols, mRows);
      cv::Mat w, imgw;
      createHanningWindow(w, img.size(), GetMatType<Float>());
      multiply(img, w, imgw);
      cv::Mat w0 = w.clone();
      cv::Mat r0 = cv::Mat::ones(w.size(), GetMatType<Float>());
      copyMakeBorder(w0, w0, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, cv::BORDER_CONSTANT, cv::Scalar::all(0));
      copyMakeBorder(r0, r0, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, cv::BORDER_CONSTANT, cv::Scalar::all(0));

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
      cv::Mat bpR = cv::Mat::zeros(mRows, mCols, GetMatType<Float>());
      cv::Mat bpG = cv::Mat::zeros(mRows, mCols, GetMatType<Float>());
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
        normalize(bpG, bpG, 0.0, 1.0, cv::NORM_MINMAX);

      cv::Mat bpR0, bpG0;
      copyMakeBorder(bpR, bpR0, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, cv::BORDER_CONSTANT, cv::Scalar::all(0));
      copyMakeBorder(bpG, bpG0, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, cv::BORDER_CONSTANT, cv::Scalar::all(0));

      // Plot1D::Plot(GetIota(bpR0.cols, 1), {GetMidRow(bpR0), GetMidRow(bpG0)}, "b0", "x", "filter", {"cv::Rect", "Gauss"}, Plot::pens, mDebugDirectory + "/1DBandpass.png");
      // Plot1D::Plot(GetIota(bpR0.cols, 1), {GetMidRow(Fourier::ifftlogmagn(bpR0)), GetMidRow(Fourier::ifftlogmagn(bpG0))}, "b1", "fx", "log IDFT", {"cv::Rect", "Gauss"}, Plot::pens, mDebugDirectory
      // +
      // "/1DBandpassIDFT.png"); Plot2D::Plot(bpR, "b2", "x", "y", "filter", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassR.png"); Plot2D::Plot(bpG, "b3", "x", "y", "filter", 0, 1, 0, 1, 0,
      // mDebugDirectory + "/2DBandpassG.png"); Plot2D::Plot(Fourier::ifftlogmagn(bpR0, 10), "b4", "fx", "fy", "log IDFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassRIDFT.png");
      // Plot2D::Plot(Fourier::ifftlogmagn(bpG0, 10), "b5", "fx", "fy", "log IDFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassGIDFT.png");
    }

    if constexpr (debugBandpassRinging)
    {
      cv::Mat img = roicrop(LoadUnitFloatImage<Float>("../data/test.png"), 4098 / 2, 4098 / 2, mCols, mRows);
      cv::Mat fftR = Fourier::fft(img);
      cv::Mat fftG = Fourier::fft(img);
      cv::Mat filterR = cv::Mat::zeros(img.size(), GetMatType<Float>());
      cv::Mat filterG = cv::Mat::zeros(img.size(), GetMatType<Float>());

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

      multiply(fftR, filterRc, fftR);
      multiply(fftG, filterGc, fftG);

      cv::Mat imgfR = Fourier::ifft(fftR);
      cv::Mat imgfG = Fourier::ifft(fftG);

      normalize(imgfR, imgfR, 0.0, 1.0, cv::NORM_MINMAX);
      normalize(imgfG, imgfG, 0.0, 1.0, cv::NORM_MINMAX);

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

private:
  i32 mRows = 0;
  i32 mCols = 0;
  f64 mBandpassL = 0;
  f64 mBandpassH = 1;
  i32 mL2size = 7;
  f64 mL1ratio = 0.5;
  f64 mL1ratioStep = 0.05;
  i32 mUpsampleCoeff = 15;
  i32 mMaxIterations = 10;
  BandpassType mBandpassType = BandpassType::Gaussian;
  InterpolationType mInterpolationType = InterpolationType::Linear;
  WindowType mWindowType = WindowType::Hann;
  cv::Mat mBandpass;
  cv::Mat mWindow;
  cv::Mat mL1circle;
  mutable std::string mDebugName = "IPC";
  mutable std::string mDebugDirectory;
  mutable i32 mDebugIndex = 0;
  mutable cv::Point2d mDebugTrueShift;

  void UpdateWindow()
  {
    PROFILE_FUNCTION;
    switch (mWindowType)
    {
    case WindowType::Hann:
      createHanningWindow(mWindow, cv::Size(mCols, mRows), GetMatType<Float>());
      return;
    default:
      return;
    }
  }

  void UpdateL1circle()
  {
    PROFILE_FUNCTION;
    mL1circle = kirkl<Float>(GetL1size(mL2size * mUpsampleCoeff, mL1ratio));
  }

  void UpdateBandpass()
  {
    PROFILE_FUNCTION;
    mBandpass = cv::Mat::ones(mRows, mCols, GetMatType<Float>());

    if (mBandpassType == BandpassType::None)
      return;
    if (mBandpassL == 0 and mBandpassH == 0)
      return;

    switch (mBandpassType)
    {
    case BandpassType::Gaussian:
      if (mBandpassL == 0 and mBandpassH != 0)
      {
        for (i32 r = 0; r < mRows; ++r)
        {
          auto bpp = mBandpass.ptr<Float>(r);
          for (i32 c = 0; c < mCols; ++c)
            bpp[c] = LowpassEquation(r, c);
        }
      }
      else if (mBandpassL != 0 and mBandpassH == 0)
      {
        for (i32 r = 0; r < mRows; ++r)
        {
          auto bpp = mBandpass.ptr<Float>(r);
          for (i32 c = 0; c < mCols; ++c)
            bpp[c] = HighpassEquation(r, c);
        }
      }
      else if (mBandpassL != 0 and mBandpassH != 0)
      {
        for (i32 r = 0; r < mRows; ++r)
        {
          auto bpp = mBandpass.ptr<Float>(r);
          for (i32 c = 0; c < mCols; ++c)
            bpp[c] = BandpassGEquation(r, c);
        }
        normalize(mBandpass, mBandpass, 0.0, 1.0, cv::NORM_MINMAX);
      }
      break;
    case BandpassType::Rectangular:
      if (mBandpassL < mBandpassH)
      {
        for (i32 r = 0; r < mRows; ++r)
        {
          auto bpp = mBandpass.ptr<Float>(r);
          for (i32 c = 0; c < mCols; ++c)
            bpp[c] = BandpassREquation(r, c);
        }
      }
      break;
    default:
      return;
    }

    Fourier::ifftshift(mBandpass);
  }
  f64 LowpassEquation(i32 row, i32 col) const
  {
    return std::exp(-1.0 / (2. * std::pow(mBandpassH, 2)) * (std::pow(col - mCols / 2, 2) / std::pow(mCols / 2, 2) + std::pow(row - mRows / 2, 2) / std::pow(mRows / 2, 2)));
  }
  f64 HighpassEquation(i32 row, i32 col) const
  {
    return 1.0 - std::exp(-1.0 / (2. * std::pow(mBandpassL, 2)) * (std::pow(col - mCols / 2, 2) / std::pow(mCols / 2, 2) + std::pow(row - mRows / 2, 2) / std::pow(mRows / 2, 2)));
  }
  f64 BandpassGEquation(i32 row, i32 col) const { return LowpassEquation(row, col) * HighpassEquation(row, col); }
  f64 BandpassREquation(i32 row, i32 col) const
  {
    f64 r = std::sqrt(0.5 * (std::pow(col - mCols / 2, 2) / std::pow(mCols / 2, 2) + std::pow(row - mRows / 2, 2) / std::pow(mRows / 2, 2)));
    return (mBandpassL <= r and r <= mBandpassH) ? 1 : 0;
  }
  template <bool DebugMode, bool Normalize = false>
  static void ConvertToUnitFloat(cv::Mat& image)
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::ConvertToUnitFloat");

    if (image.type() != GetMatType<Float>())
      image.convertTo(image, GetMatType<Float>());

    if constexpr (Normalize)
      normalize(image, image, 0, 1, cv::NORM_MINMAX);
  }
  template <bool DebugMode>
  void ApplyWindow(cv::Mat& image) const
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::ApplyWindow");

    if (mWindowType != WindowType::None)
      multiply(image, mWindow, image);
  }
  template <bool DebugMode>
  static cv::Mat CalculateFourierTransform(cv::Mat&& image)
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::CalculateFourierTransform");
    return Fourier::fft(std::move(image));
  }
  template <bool DebugMode, bool CrossCorrelation>
  cv::Mat CalculateCrossPowerSpectrum(cv::Mat&& dft1, cv::Mat&& dft2) const
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::CalculateCrossPowerSpectrum");

    for (i32 row = 0; row < dft1.rows; ++row)
    {
      auto dft1p = dft1.ptr<cv::Vec<Float, 2>>(row);
      auto dft2p = dft2.ptr<cv::Vec<Float, 2>>(row);
      auto bandp = mBandpass.ptr<Float>(row);
      for (i32 col = 0; col < dft1.cols; ++col)
      {
        const Float re = dft1p[col][0] * dft2p[col][0] + dft1p[col][1] * dft2p[col][1];
        const Float im = dft1p[col][0] * dft2p[col][1] - dft1p[col][1] * dft2p[col][0];
        const Float mag = std::sqrt(re * re + im * im);
        const Float band = bandp[col];

        if constexpr (CrossCorrelation)
        {
          // reuse dft1 memory
          dft1p[col][0] = re * band;
          dft1p[col][1] = im * band;
        }
        else
        {
          // reuse dft1 memory
          dft1p[col][0] = re / mag * band;
          dft1p[col][1] = im / mag * band;
        }
      }
    }
    return dft1;
  }
  template <bool DebugMode>
  static cv::Mat CalculateL3(cv::Mat&& crosspower)
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::CalculateL3");
    cv::Mat L3 = Fourier::ifft(std::move(crosspower));
    Fourier::fftshift(L3);
    return L3;
  }
  template <bool DebugMode>
  static cv::Point2d GetPeak(const cv::Mat& mat)
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::GetPeak");
    cv::Point2i peak(0, 0);
    minMaxLoc(mat, nullptr, nullptr, nullptr, &peak);
    return peak;
  }
  template <bool Circular>
  static cv::Point2d GetPeakSubpixel(const cv::Mat& mat, const cv::Mat& L1circle)
  {
    PROFILE_FUNCTION;
    if constexpr (Circular)
    {
      const auto m = cv::moments(mat.mul(L1circle));
      return cv::Point2d(m.m10 / m.m00, m.m01 / m.m00);
    }
    else
    {
      const auto m = cv::moments(mat);
      return cv::Point2d(m.m10 / m.m00, m.m01 / m.m00);
    }
  }
  template <bool DebugMode>
  static cv::Mat CalculateL2(const cv::Mat& L3, const cv::Point2d& L3peak, i32 L2size)
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::CalculateL2");
    return roicrop(L3, L3peak.x, L3peak.y, L2size, L2size);
  }
  template <bool DebugMode>
  cv::Mat CalculateL2U(const cv::Mat& L2) const
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::CalculateL2U");

    cv::Mat L2U;
    switch (mInterpolationType)
    {
    case InterpolationType::NearestNeighbor:
      cv::resize(L2, L2U, L2.size() * mUpsampleCoeff, 0, 0, cv::INTER_NEAREST);
      break;
    case InterpolationType::Linear:
      cv::resize(L2, L2U, L2.size() * mUpsampleCoeff, 0, 0, cv::INTER_LINEAR);
      break;
    case InterpolationType::Cubic:
      cv::resize(L2, L2U, L2.size() * mUpsampleCoeff, 0, 0, cv::INTER_CUBIC);
      break;
    default:
      break;
    }

    return L2U;
  }
  static i32 GetL1size(i32 L2Usize, f64 L1ratio)
  {
    i32 L1size = std::floor(L1ratio * L2Usize);
    L1size = L1size % 2 ? L1size : L1size + 1;
    return L1size;
  }
  static cv::Mat CalculateL1(const cv::Mat& L2U, const cv::Point2d& L2Upeak, i32 L1size) { return roicropref(L2U, L2Upeak.x, L2Upeak.y, L1size, L1size); }
  static bool IsOutOfBounds(const cv::Point2i& peak, const cv::Mat& mat, i32 size) { return IsOutOfBounds(peak, mat, {size, size}); }
  static bool IsOutOfBounds(const cv::Point2i& peak, const cv::Mat& mat, cv::Size size)
  {
    return peak.x - size.width / 2 < 0 or peak.y - size.height / 2 < 0 or peak.x + size.width / 2 >= mat.cols or peak.y + size.height / 2 >= mat.rows;
  }
  static bool AccuracyReached(const cv::Point2d& L1peak, const cv::Point2d& L1mid) { return std::abs(L1peak.x - L1mid.x) < 0.5 and std::abs(L1peak.y - L1mid.y) < 0.5; }
  template <bool DebugMode>
  static bool ReduceL2size(i32& L2size)
  {
    L2size -= 2;
    if constexpr (DebugMode)
      LOG_WARNING("L2 out of bounds - reducing L2size to {}", L2size);
    return L2size >= 3;
  }
  template <bool DebugMode>
  void ReduceL1ratio(f64& L1ratio) const
  {
    if constexpr (DebugMode)
      LOG_WARNING("L1 did not converge - reducing L1ratio: {:.2f} -> {:.2f}", L1ratio, L1ratio - mL1ratioStep);
    L1ratio -= mL1ratioStep;
  }
  static cv::Point2d GetPixelShift(const cv::Point2d& L3peak, const cv::Point2d& L3mid) { return L3peak - L3mid; }
  template <bool DebugMode>
  cv::Point2d GetSubpixelShift(const cv::Mat& L3, const cv::Point2d& L3peak, const cv::Point2d& L3mid, i32 L2size) const
  {
    PROFILE_FUNCTION;
    while (IsOutOfBounds(L3peak, L3, L2size))
      if (!ReduceL2size<DebugMode>(L2size))
        return L3peak - L3mid;

    cv::Mat L2 = CalculateL2<DebugMode>(L3, L3peak, 5);
    cv::Point2d L2peak = GetPeakSubpixel<false>(L2, cv::Mat());
    cv::Point2d L2mid(L2.cols / 2, L2.rows / 2);
    return L3peak - L3mid + L2peak - L2mid;
  }

  void DebugInputImages(const cv::Mat& image1, const cv::Mat& image2) const
  {
    PyPlot::Plot(fmt::format("{} I1", mDebugName), {.z = image1, .cmap = "gray"});
    PyPlot::Plot(fmt::format("{} I2", mDebugName), {.z = image2, .cmap = "gray"});
  }

  void DebugFourierTransforms(const cv::Mat& dft1, const cv::Mat& dft2) const
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

  void DebugCrossPowerSpectrum(const cv::Mat& crosspower) const
  {
    PyPlot::Plot(fmt::format("{} CP magnitude", mDebugName), {.z = Fourier::fftshift(Fourier::magn(crosspower))});
    PyPlot::Plot(fmt::format("{} CP magnitude 1D", mDebugName), {.y = GetMidRow<Float>(Fourier::fftshift(Fourier::magn(crosspower)))});
    PyPlot::Plot(fmt::format("{} CP phase", mDebugName), {.z = Fourier::fftshift(Fourier::phase(crosspower))});
    PyPlot::Plot(fmt::format("{} CP sawtooth", mDebugName),
        {.ys = {GetRow<Float>(Fourier::fftshift(Fourier::phase(crosspower)), 0.6 * crosspower.rows), GetCol<Float>(Fourier::fftshift(Fourier::phase(crosspower)), 0.6 * crosspower.cols)},
            .label_ys = {"x", "y"}});
  }

  void DebugL3(const cv::Mat& L3) const { PyPlot::Plot(fmt::format("{} L3", mDebugName), {.z = L3}); }

  void DebugL2(const cv::Mat& L2) const
  {
    auto plot = L2.clone();
    resize(plot, plot, plot.size() * mUpsampleCoeff, 0, 0, cv::INTER_NEAREST);
    PyPlot::Plot(fmt::format("{} L2", mDebugName), {.z = plot});
  }

  void DebugL2U(const cv::Mat& L2, const cv::Mat& L2U) const
  {
    PyPlot::Plot(fmt::format("{} L2U", mDebugName), {.z = L2U});

    if (0)
    {
      cv::Mat nearest, linear, cubic;
      resize(L2, nearest, L2.size() * mUpsampleCoeff, 0, 0, cv::INTER_NEAREST);
      resize(L2, linear, L2.size() * mUpsampleCoeff, 0, 0, cv::INTER_LINEAR);
      resize(L2, cubic, L2.size() * mUpsampleCoeff, 0, 0, cv::INTER_CUBIC);

      PyPlot::Plot("IPCL2UN", {.z = nearest});
      PyPlot::Plot("IPCL2UL", {.z = linear});
      PyPlot::Plot("IPCL2UC", {.z = cubic});
    }
  }

  void DebugL1B(const cv::Mat& L2U, i32 L1size, const cv::Point2d& L3shift) const
  {
    cv::Mat mat = CalculateL1(L2U, cv::Point(L2U.cols / 2, L2U.rows / 2), L1size).clone();
    cv::normalize(mat, mat, 0, 1, cv::NORM_MINMAX); // for black crosshairs + cross
    mat = mat.mul(kirkl<Float>(mat.rows));
    DrawCrosshairs(mat);
    DrawCross(mat, cv::Point2d(mat.cols / 2, mat.rows / 2) + mUpsampleCoeff * (mDebugTrueShift - L3shift));
    cv::resize(mat, mat, cv::Size(mUpsampleCoeff * mL2size, mUpsampleCoeff * mL2size), 0, 0, cv::INTER_NEAREST);
    PyPlot::Plot(fmt::format("{} L1B", mDebugName), {.z = mat, .save = not mDebugDirectory.empty() ? fmt::format("{}/L1B_{}.png", mDebugDirectory, mDebugIndex) : ""});
  }

  void DebugL1A(const cv::Mat& L1, const cv::Point2d& L3shift, const cv::Point2d& L2Ushift, bool last = false) const
  {
    cv::Mat mat = L1.clone();
    cv::normalize(mat, mat, 0, 1, cv::NORM_MINMAX); // for black crosshairs + cross
    mat = mat.mul(kirkl<Float>(mat.rows));
    DrawCrosshairs(mat);
    DrawCross(mat, cv::Point2d(mat.cols / 2, mat.rows / 2) + mUpsampleCoeff * (mDebugTrueShift - L3shift) - L2Ushift);
    cv::resize(mat, mat, cv::Size(mUpsampleCoeff * mL2size, mUpsampleCoeff * mL2size), 0, 0, cv::INTER_NEAREST);
    PyPlot::Plot(fmt::format("{} L1A", mDebugName), {.z = mat, .save = not mDebugDirectory.empty() and last ? fmt::format("{}/L1A_{}.png", mDebugDirectory, mDebugIndex) : ""});
  }

  static cv::Mat ColorComposition(const cv::Mat& img1, const cv::Mat& img2)
  {
    PROFILE_FUNCTION;
    const cv::Vec<Float, 3> img1clr = {1, 0.5, 0};
    const cv::Vec<Float, 3> img2clr = {0, 0.5, 1};

    const f64 gamma1 = 1.0;
    const f64 gamma2 = 1.0;

    cv::Mat img1c = cv::Mat::zeros(img1.size(), GetMatType<Float>(3));
    cv::Mat img2c = cv::Mat::zeros(img2.size(), GetMatType<Float>(3));

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
      normalize(img1c, img1c, 0, 1, cv::NORM_MINMAX);
      normalize(img2c, img2c, 0, 1, cv::NORM_MINMAX);
    }

    return (img1c + img2c) / 2;
  }

  static std::vector<cv::Mat> LoadImages(const std::string& imagesDirectory)
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
      image = roicropmid(image, cropFocusRatio * image.cols, cropFocusRatio * image.rows);
      images.push_back(image);
      LOG_DEBUG("Loaded image {}", path);
    }
    return images;
  }

  std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>> CreateImagePairs(const std::vector<cv::Mat>& images, f64 maxShift, i32 itersPerImage, f64 noiseStdev) const
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
        cv::Point2d shift(rand11() * maxShift, rand11() * maxShift);
        cv::Point2i point(clamp(rand01() * image.cols, mCols, image.cols - mCols), clamp(rand01() * image.rows, mRows, image.rows - mRows));
        cv::Mat Tmat = (cv::Mat_<f64>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
        cv::Mat imageShifted;
        warpAffine(image, imageShifted, Tmat, image.size());
        cv::Mat image1 = roicrop(image, point.x, point.y, mCols, mRows);
        cv::Mat image2 = roicrop(imageShifted, point.x, point.y, mCols, mRows);

        ConvertToUnitFloat<false>(image1);
        ConvertToUnitFloat<false>(image2);

        AddNoise(image1, noiseStdev);
        AddNoise(image2, noiseStdev);

        imagePairs.push_back({image1, image2, shift});

        if constexpr (0)
        {
          cv::Mat hcct;
          hconcat(image1, image2, hcct);
          showimg(hcct, fmt::format("IPC optimization pair {}", i));
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

  static void AddNoise(cv::Mat& image, f64 noiseStdev)
  {
    PROFILE_FUNCTION;
    if (noiseStdev <= 0)
      return;

    cv::Mat noise = cv::Mat::zeros(image.rows, image.cols, GetMatType<Float>());
    randn(noise, 0, noiseStdev);
    image += noise;
  }

  IterativePhaseCorrelation<Float> CreateIPCFromParams(const std::vector<f64>& params) const
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

  std::function<f64(const std::vector<f64>&)> CreateObjectiveFunction(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs) const
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

  std::function<f64(const std::vector<f64>&)> CreateObjectiveFunction(const std::function<f64(const IterativePhaseCorrelation&)>& obj) const
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

  static std::vector<f64> CalculateOptimalParameters(const std::function<f64(const std::vector<f64>&)>& obj, const std::function<f64(const std::vector<f64>&)>& valid, i32 populationSize)
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

  void ApplyOptimalParameters(const std::vector<f64>& optimalParameters)
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

  static void ShowOptimizationPlots(const std::vector<cv::Point2d>& shiftsReference, const std::vector<cv::Point2d>& shiftsPixel, const std::vector<cv::Point2d>& shiftsNonit,
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

  std::vector<cv::Point2d> GetShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs) const
  {
    PROFILE_FUNCTION;
    std::vector<cv::Point2d> out;
    out.reserve(imagePairs.size());

    for (const auto& [image1, image2, referenceShift] : imagePairs)
      out.push_back(Calculate(image1, image2));

    return out;
  }

  std::vector<cv::Point2d> GetNonIterativeShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs) const
  {
    PROFILE_FUNCTION;
    std::vector<cv::Point2d> out;
    out.reserve(imagePairs.size());

    for (const auto& [image1, image2, referenceShift] : imagePairs)
      out.push_back(Calculate<false, false, AccuracyType::Subpixel>(image1, image2));

    return out;
  }

  std::vector<cv::Point2d> GetPixelShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs) const
  {
    PROFILE_FUNCTION;
    std::vector<cv::Point2d> out;
    out.reserve(imagePairs.size());

    for (const auto& [image1, image2, referenceShift] : imagePairs)
      out.push_back(Calculate<false, false, AccuracyType::Pixel>(image1, image2));

    return out;
  }

  static std::vector<cv::Point2d> GetReferenceShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs)
  {
    PROFILE_FUNCTION;
    std::vector<cv::Point2d> out;
    out.reserve(imagePairs.size());

    for (const auto& [image1, image2, referenceShift] : imagePairs)
      out.push_back(referenceShift);

    return out;
  }

  static f64 GetAverageAccuracy(const std::vector<cv::Point2d>& shiftsReference, const std::vector<cv::Point2d>& shifts)
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

  static void ShowRandomImagePair(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs)
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION("ShowRandomImagePair");

    const auto& [img1, img2, shift] = imagePairs[static_cast<usize>(rand01() * imagePairs.size())];
    cv::Mat concat;
    hconcat(img1, img2, concat);
    Plot2D::Set("Random image pair");
    Plot2D::SetColorMapType(QCPColorGradient::gpGrayscale);
    Plot2D::Plot("Random image pair", concat);
  }
};
