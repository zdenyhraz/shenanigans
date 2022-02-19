#pragma once
#include "Fourier/Fourier.h"

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
    mBandpassL = clamp(bandpassL, -std::numeric_limits<f64>::max(), 1); // L from [-inf, 1]
    mBandpassH = clamp(bandpassH, 0, std::numeric_limits<f64>::max());  // H from [0, inf]
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
        [[unlikely]] L1circle = kirkl(L1size);
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
  cv::Mat Align(cv::Mat&& image1, cv::Mat&& image2) const;
  std::tuple<cv::Mat, cv::Mat> CalculateFlow(const cv::Mat& image1, const cv::Mat& image2, f32 resolution) const { return CalculateFlow(image1.clone(), image2.clone(), resolution); }
  std::tuple<cv::Mat, cv::Mat> CalculateFlow(cv::Mat&& image1, cv::Mat&& image2, f32 resolution) const;
  void ShowDebugStuff() const;
  void Optimize(const std::function<f64(const IterativePhaseCorrelation&)>& obj, i32 populationSize = OptimizedParameterCount * 7);
  void Optimize(const std::string& trainingImagesDirectory, const std::string& validationImagesDirectory, f32 maxShift = 2.0, f32 noiseStdev = 0.01, i32 itersPerImage = 100, f64 validationRatio = 0.2,
      i32 populationSize = OptimizedParameterCount * 7);
  void PlotObjectiveFunctionLandscape(const std::string& trainingImagesDirectory, f32 maxShift, f32 noiseStdev, i32 itersPerImage, i32 iters) const;
  void PlotImageSizeAccuracyDependence(const std::string& trainingImagesDirectory, f32 maxShift, f32 noiseStdev, i32 itersPerImage, i32 iters);
  void PlotUpsampleCoefficientAccuracyDependence(const std::string& trainingImagesDirectory, f32 maxShift, f32 noiseStdev, i32 itersPerImage, i32 iters) const;
  void PlotNoiseAccuracyDependence(const std::string& trainingImagesDirectory, f32 maxShift, f32 noiseStdev, i32 itersPerImage, i32 iters) const;
  void PlotNoiseOptimalBPHDependence(const std::string& trainingImagesDirectory, f32 maxShift, f32 noiseStdev, i32 itersPerImage, i32 iters) const;
  static std::string BandpassType2String(BandpassType type);
  static std::string InterpolationType2String(InterpolationType type);
  static std::string WindowType2String(WindowType type);

private:
  i32 mRows = 0;
  i32 mCols = 0;
  f64 mBandpassL = 0;
  f64 mBandpassH = 1;
  i32 mL2size = 7;
  f64 mL1ratio = 0.5;
  f64 mL1ratioStep = 0.1;
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
      createHanningWindow(mWindow, cv::Size(mCols, mRows), CV_32F);
      return;
    default:
      return;
    }
  }

  void UpdateL1circle()
  {
    PROFILE_FUNCTION;
    mL1circle = kirkl(GetL1size(mL2size * mUpsampleCoeff, mL1ratio));
  }

  void UpdateBandpass()
  {
    PROFILE_FUNCTION;
    mBandpass = cv::Mat::ones(mRows, mCols, CV_32F);
    if (mBandpassType == BandpassType::None)
      return;

    switch (mBandpassType)
    {
    case BandpassType::Gaussian:
      if (mBandpassL <= 0 and mBandpassH < 1)
      {
        for (i32 r = 0; r < mRows; ++r)
          for (i32 c = 0; c < mCols; ++c)
            mBandpass.at<f32>(r, c) = LowpassEquation(r, c);
      }
      else if (mBandpassL > 0 and mBandpassH >= 1)
      {
        for (i32 r = 0; r < mRows; ++r)
          for (i32 c = 0; c < mCols; ++c)
            mBandpass.at<f32>(r, c) = HighpassEquation(r, c);
      }
      else if (mBandpassL > 0 and mBandpassH < 1)
      {
        for (i32 r = 0; r < mRows; ++r)
          for (i32 c = 0; c < mCols; ++c)
            mBandpass.at<f32>(r, c) = BandpassGEquation(r, c);

        normalize(mBandpass, mBandpass, 0.0, 1.0, cv::NORM_MINMAX);
      }
      break;
    case BandpassType::Rectangular:
      if (mBandpassL < mBandpassH)
      {
        for (i32 r = 0; r < mRows; ++r)
          for (i32 c = 0; c < mCols; ++c)
            mBandpass.at<f32>(r, c) = BandpassREquation(r, c);
      }
      break;
    default:
      return;
    }

    Fourier::ifftshift(mBandpass);
  }
  f32 LowpassEquation(i32 row, i32 col) const
  {
    return exp(-1.0 / (2. * std::pow(mBandpassH, 2)) * (std::pow(col - mCols / 2, 2) / std::pow(mCols / 2, 2) + std::pow(row - mRows / 2, 2) / std::pow(mRows / 2, 2)));
  }
  f32 HighpassEquation(i32 row, i32 col) const
  {
    return 1.0 - exp(-1.0 / (2. * std::pow(mBandpassL, 2)) * (std::pow(col - mCols / 2, 2) / std::pow(mCols / 2, 2) + std::pow(row - mRows / 2, 2) / std::pow(mRows / 2, 2)));
  }
  f32 BandpassGEquation(i32 row, i32 col) const { return LowpassEquation(row, col) * HighpassEquation(row, col); }
  f32 BandpassREquation(i32 row, i32 col) const
  {
    f64 r = sqrt(0.5 * (std::pow(col - mCols / 2, 2) / std::pow(mCols / 2, 2) + std::pow(row - mRows / 2, 2) / std::pow(mRows / 2, 2)));
    return (mBandpassL <= r and r <= mBandpassH) ? 1 : 0;
  }
  template <bool DebugMode, bool Normalize = false>
  static void ConvertToUnitFloat(cv::Mat& image)
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::ConvertToUnitFloat");

    if (image.type() != CV_32F)
      [[unlikely]] image.convertTo(image, CV_32F);

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
      auto dft1p = dft1.ptr<cv::Vec2f>(row);
      auto dft2p = dft2.ptr<cv::Vec2f>(row);
      auto bandp = mBandpass.ptr<f32>(row);
      for (i32 col = 0; col < dft1.cols; ++col)
      {
        const f32 re = dft1p[col][0] * dft2p[col][0] + dft1p[col][1] * dft2p[col][1];
        const f32 im = dft1p[col][0] * dft2p[col][1] - dft1p[col][1] * dft2p[col][0];
        const f32 mag = std::sqrt(re * re + im * im);
        const f32 band = bandp[col];

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
      cv::Moments m = cv::moments(mat.mul(L1circle));
      return cv::Point2d(m.m10 / m.m00, m.m01 / m.m00);
    }
    else
    {
      cv::Moments m = cv::moments(mat);
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

  void DebugInputImages(const cv::Mat& image1, const cv::Mat& image2) const;
  void DebugFourierTransforms(const cv::Mat& dft1, const cv::Mat& dft2) const;
  void DebugCrossPowerSpectrum(const cv::Mat& crosspower) const;
  void DebugL3(const cv::Mat& L3) const;
  void DebugL2(const cv::Mat& L2) const;
  void DebugL2U(const cv::Mat& L2, const cv::Mat& L2U) const;
  void DebugL1B(const cv::Mat& L2U, i32 L1size, const cv::Point2d& L3shift) const;
  void DebugL1A(const cv::Mat& L1, const cv::Point2d& L3shift, const cv::Point2d& L2Ushift, bool last = false) const;
  std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>> CreateImagePairs(const std::vector<cv::Mat>& images, f64 maxShift, i32 itersPerImage, f64 noiseStdev) const;
  IterativePhaseCorrelation CreateIPCFromParams(const std::vector<f64>& params) const;
  std::function<f64(const std::vector<f64>&)> CreateObjectiveFunction(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs) const;
  std::function<f64(const std::vector<f64>&)> CreateObjectiveFunction(const std::function<f64(const IterativePhaseCorrelation&)>& obj) const;
  void ApplyOptimalParameters(const std::vector<f64>& optimalParameters);
  std::vector<cv::Point2d> GetNonIterativeShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs) const;
  std::vector<cv::Point2d> GetShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs) const;
  std::vector<cv::Point2d> GetPixelShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs) const;
  static std::vector<f64> CalculateOptimalParameters(const std::function<f64(const std::vector<f64>&)>& obj, const std::function<f64(const std::vector<f64>&)>& valid, i32 populationSize);
  static void ShowOptimizationPlots(const std::vector<cv::Point2d>& shiftsReference, const std::vector<cv::Point2d>& shiftsPixel, const std::vector<cv::Point2d>& shiftsNonit,
      const std::vector<cv::Point2d>& shiftsBefore, const std::vector<cv::Point2d>& shiftsAfter);
  static std::vector<cv::Point2d> GetReferenceShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs);
  static std::vector<cv::Mat> LoadImages(const std::string& imagesDirectory);
  static cv::Mat ColorComposition(const cv::Mat& img1, const cv::Mat& img2);
  static f64 GetAverageAccuracy(const std::vector<cv::Point2d>& shiftsReference, const std::vector<cv::Point2d>& shifts);
  static void AddNoise(cv::Mat& image, f64 noiseStdev);
  static void ShowRandomImagePair(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs);
};
