#pragma once
#include "Fourier/Fourier.hpp"
#include "Optimization/Evolution.hpp"
#include "UtilsCV/Vectmat.hpp"

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

  explicit IterativePhaseCorrelation(i32 rows, i32 cols = -1, f64 bpL = 0, f64 bpH = 1) { Initialize(rows, cols, bpL, bpH); }
  explicit IterativePhaseCorrelation(const cv::Size& size, f64 bpL = 0, f64 bpH = 1) { Initialize(size.height, size.width, bpL, bpH); }
  explicit IterativePhaseCorrelation(const cv::Mat& img, f64 bpL = 0, f64 bpH = 1) { Initialize(img.rows, img.cols, bpL, bpH); }

  void Initialize(i32 rows, i32 cols, f64 bpL, f64 bpH)
  {
    PROFILE_FUNCTION;
    SetSize(rows, cols);
    SetBandpassParameters(bpL, bpH);
    UpdateL1circle();
  }

  void SetSize(i32 rows, i32 cols = -1)
  {
    PROFILE_FUNCTION;
    mRows = rows;
    mCols = cols > 0 ? cols : rows;

    if (mWin.rows != mRows or mWin.cols != mCols)
      UpdateWindow();

    if (mBP.rows != mRows or mBP.cols != mCols)
      UpdateBandpass();
  }

  void SetSize(const cv::Size& size) { SetSize(size.height, size.width); }

  void SetBandpassParameters(f64 bpL, f64 bpH)
  {
    mBPL = std::clamp(bpL, 0., std::numeric_limits<f64>::max());
    mBPH = std::clamp(bpH, 0., std::numeric_limits<f64>::max());
    UpdateBandpass();
  }

  void SetBandpassType(BandpassType type)
  {
    mBPT = type;
    UpdateBandpass();
  }

  void SetL2size(i32 L2size)
  {
    mL2size = (L2size % 2) ? L2size : L2size + 1;
    UpdateL1circle();
  }

  void SetL1ratio(f64 L1ratio)
  {
    mL1ratio = L1ratio;
    UpdateL1circle();
  }

  void SetUpsampleCoeff(i32 upsampleCoeff)
  {
    mUC = (upsampleCoeff % 2) ? upsampleCoeff : upsampleCoeff + 1;
    UpdateL1circle();
  }

  void SetMaxIterations(i32 maxIterations) { mMaxIter = maxIterations; }

  void SetInterpolationType(InterpolationType interpolationType) { mIntT = interpolationType; }

  void SetWindowType(WindowType type)
  {
    mWinT = type;
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
  f64 GetBandpassL() const { return mBPL; }
  f64 GetBandpassH() const { return mBPH; }
  i32 GetL2size() const { return mL2size; }
  f64 GetL1ratio() const { return mL1ratio; }
  i32 GetUpsampleCoeff() const { return mUC; }
  cv::Mat GetWindow() const { return mWin; }
  cv::Mat GetBandpass() const { return mBP; }
  BandpassType GetBandpassType() const { return mBPT; }
  WindowType GetWindowType() const { return mWinT; }
  InterpolationType GetInterpolationType() const { return mIntT; }

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
    if constexpr (DebugMode and not CrossCorrelation)
      DebugL2(L2);

    // L2U
    cv::Mat L2U = CalculateL2U<DebugMode>(L2);
    cv::Point2d L2Umid(L2U.cols / 2, L2U.rows / 2);
    if constexpr (DebugMode)
      DebugL2U(L2, L2U);

    // L1
    i32 L1size;
    cv::Mat L1, L1circle;
    cv::Point2d L2Upeak, L1mid, L1peak;

    for (f64 L1ratio = mL1ratio; GetL1size(L2U.cols, L1ratio) > 0; L1ratio -= mL1ratioStep)
    {
      PROFILE_SCOPE(IterativeRefinement);
      if constexpr (DebugMode)
        LOG_DEBUG("Iterative refinement L1ratio: {:.2f}", L1ratio);

      L2Upeak = L2Umid; // reset L2U peak position
      L1size = GetL1size(L2U.cols, L1ratio);
      L1mid = cv::Point2d(L1size / 2, L1size / 2);
      L1circle = mL1circle.cols == L1size ? mL1circle : Kirkl<Float>(L1size);

      if constexpr (DebugMode)
        DebugL1B(L2U, L1size, L3peak - L3mid);

      for (i32 iter = 0; iter < mMaxIter; ++iter)
      {
        PROFILE_SCOPE(IterativeRefinementIteration);
        if constexpr (DebugMode)
          LOG_DEBUG("Iterative refinement {} L2Upeak: {}", iter, L2Upeak);
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
            return L3peak - L3mid + (L2Upeak - L2Umid + L1peak - L1mid) / mUC;
          }
      }

      if constexpr (DebugMode)
        LOG_WARNING("L1 did not converge - reducing L1ratio: {:.2f} -> {:.2f}", L1ratio, L1ratio - mL1ratioStep);
    }

    if constexpr (DebugMode)
      LOG_WARNING("L1 failed to converge with all L1ratios, return non-iterative subpixel shift");
    return GetSubpixelShift<DebugMode>(L3, L3peak, L3mid, L2size);
  }

  cv::Mat Align(const cv::Mat& image1, const cv::Mat& image2) const;
  cv::Mat Align(cv::Mat&& image1, cv::Mat&& image2) const;
  std::tuple<cv::Mat, cv::Mat> CalculateFlow(const cv::Mat& image1, const cv::Mat& image2, f64 resolution) const;
  std::tuple<cv::Mat, cv::Mat> CalculateFlow(cv::Mat&& image1, cv::Mat&& image2, f64 resolution) const;
  void Optimize(const std::string& trainingImagesDirectory, const std::string& validationImagesDirectory, f64 maxShift = 2.0, f64 noiseStdev = 0.01, i32 itersPerImage = 100, f64 validationRatio = 0.2,
      i32 populationSize = OptimizedParameterCount * 7);
  void Optimize(const std::function<f64(const IterativePhaseCorrelation&)>& obj, i32 populationSize = OptimizedParameterCount * 7);
  void PlotObjectiveFunctionLandscape(const std::string& trainingImagesDirectory, f64 maxShift, f64 noiseStdev, i32 itersPerImage, i32 iters) const;
  void PlotImageSizeAccuracyDependence(const std::string& trainingImagesDirectory, f64 maxShift, f64 noiseStdev, i32 itersPerImage, i32 iters);
  void PlotUpsampleCoefficientAccuracyDependence(const std::string& trainingImagesDirectory, f64 maxShift, f64 noiseStdev, i32 itersPerImage, i32 iters) const;
  void PlotNoiseAccuracyDependence(const std::string& trainingImagesDirectory, f64 maxShift, f64 noiseStdev, i32 itersPerImage, i32 iters) const;
  void PlotNoiseOptimalBPHDependence(const std::string& trainingImagesDirectory, f64 maxShift, f64 noiseStdev, i32 itersPerImage, i32 iters) const;
  static std::string BandpassType2String(BandpassType type);
  static std::string WindowType2String(WindowType type);
  static std::string InterpolationType2String(InterpolationType type);
  void ShowDebugStuff() const;

private:
  i32 mRows = 0;
  i32 mCols = 0;
  f64 mBPL = 0;
  f64 mBPH = 1;
  i32 mL2size = 7;
  f64 mL1ratio = 0.45;
  f64 mL1ratioStep = 0.025;
  i32 mL1ratioStepCount = 4;
  i32 mUC = 51;
  i32 mMaxIter = 10;
  BandpassType mBPT = BandpassType::Gaussian;
  InterpolationType mIntT = InterpolationType::Linear;
  WindowType mWinT = WindowType::Hann;
  cv::Mat mBP;
  cv::Mat mWin;
  cv::Mat mL1circle;
  mutable std::string mDebugName = "IPC";
  mutable std::string mDebugDirectory;
  mutable i32 mDebugIndex = 0;
  mutable cv::Point2d mDebugTrueShift;

  void UpdateWindow()
  {
    PROFILE_FUNCTION;
    switch (mWinT)
    {
    case WindowType::Hann:
      cv::createHanningWindow(mWin, cv::Size(mCols, mRows), GetMatType<Float>());
      return;
    default:
      return;
    }
  }

  void UpdateL1circle() { mL1circle = Kirkl<Float>(GetL1size(mL2size * mUC, mL1ratio)); }

  void UpdateBandpass()
  {
    PROFILE_FUNCTION;
    mBP = cv::Mat::ones(mRows, mCols, GetMatType<Float>());

    if (mBPT == BandpassType::None)
      return;
    if (mBPL == 0 and mBPH == 0)
      return;

    switch (mBPT)
    {
    case BandpassType::Gaussian:
      if (mBPL == 0 and mBPH != 0)
      {
        for (i32 r = 0; r < mRows; ++r)
        {
          auto bpp = mBP.ptr<Float>(r);
          for (i32 c = 0; c < mCols; ++c)
            bpp[c] = LowpassEquation(r, c);
        }
      }
      else if (mBPL != 0 and mBPH == 0)
      {
        for (i32 r = 0; r < mRows; ++r)
        {
          auto bpp = mBP.ptr<Float>(r);
          for (i32 c = 0; c < mCols; ++c)
            bpp[c] = HighpassEquation(r, c);
        }
      }
      else if (mBPL != 0 and mBPH != 0)
      {
        for (i32 r = 0; r < mRows; ++r)
        {
          auto bpp = mBP.ptr<Float>(r);
          for (i32 c = 0; c < mCols; ++c)
            bpp[c] = BandpassGEquation(r, c);
        }
        cv::normalize(mBP, mBP, 0.0, 1.0, cv::NORM_MINMAX);
      }
      break;
    case BandpassType::Rectangular:
      if (mBPL < mBPH)
      {
        for (i32 r = 0; r < mRows; ++r)
        {
          auto bpp = mBP.ptr<Float>(r);
          for (i32 c = 0; c < mCols; ++c)
            bpp[c] = BandpassREquation(r, c);
        }
      }
      break;
    default:
      return;
    }

    Fourier::ifftshift(mBP);
  }

  f64 LowpassEquation(i32 row, i32 col) const
  {
    return std::exp(-1.0 / (2. * std::pow(mBPH, 2)) * (std::pow(col - mCols / 2, 2) / std::pow(mCols / 2, 2) + std::pow(row - mRows / 2, 2) / std::pow(mRows / 2, 2)));
  }

  f64 HighpassEquation(i32 row, i32 col) const
  {
    return 1.0 - std::exp(-1.0 / (2. * std::pow(mBPL, 2)) * (std::pow(col - mCols / 2, 2) / std::pow(mCols / 2, 2) + std::pow(row - mRows / 2, 2) / std::pow(mRows / 2, 2)));
  }

  f64 BandpassGEquation(i32 row, i32 col) const { return LowpassEquation(row, col) * HighpassEquation(row, col); }

  f64 BandpassREquation(i32 row, i32 col) const
  {
    f64 r = std::sqrt(0.5 * (std::pow(col - mCols / 2, 2) / std::pow(mCols / 2, 2) + std::pow(row - mRows / 2, 2) / std::pow(mRows / 2, 2)));
    return (mBPL <= r and r <= mBPH) ? 1 : 0;
  }

  template <bool DebugMode, bool Normalize = false>
  static void ConvertToUnitFloat(cv::Mat& image)
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::ConvertToUnitFloat");

    if (image.type() != GetMatType<Float>())
      image.convertTo(image, GetMatType<Float>());

    if constexpr (Normalize)
      cv::normalize(image, image, 0, 1, cv::NORM_MINMAX);
  }

  template <bool DebugMode>
  void ApplyWindow(cv::Mat& image) const
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::ApplyWindow");

    if (mWinT != WindowType::None)
      cv::multiply(image, mWin, image);
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
      auto dft1p = dft1.ptr<cv::Vec<Float, 2>>(row); // reuse dft1 memory
      const auto dft2p = dft2.ptr<cv::Vec<Float, 2>>(row);
      const auto bandp = mBP.ptr<Float>(row);
      for (i32 col = 0; col < dft1.cols; ++col)
      {
        const Float re = dft1p[col][0] * dft2p[col][0] + dft1p[col][1] * dft2p[col][1];
        const Float im = dft1p[col][0] * dft2p[col][1] - dft1p[col][1] * dft2p[col][0];
        const Float mag = std::sqrt(re * re + im * im);
        const Float band = bandp[col];

        if constexpr (CrossCorrelation)
        {
          dft1p[col][0] = re * band;
          dft1p[col][1] = im * band;
        }
        else
        {
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
    return RoiCrop(L3, L3peak.x, L3peak.y, L2size, L2size);
  }

  template <bool DebugMode>
  cv::Mat CalculateL2U(const cv::Mat& L2) const
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::CalculateL2U");

    cv::Mat L2U;
    switch (mIntT)
    {
    case InterpolationType::NearestNeighbor:
      cv::resize(L2, L2U, L2.size() * mUC, 0, 0, cv::INTER_NEAREST);
      break;
    case InterpolationType::Linear:
      cv::resize(L2, L2U, L2.size() * mUC, 0, 0, cv::INTER_LINEAR);
      break;
    case InterpolationType::Cubic:
      cv::resize(L2, L2U, L2.size() * mUC, 0, 0, cv::INTER_CUBIC);
      break;
    default:
      break;
    }

    return L2U;
  }

  static i32 GetL1size(i32 L2Usize, f64 L1ratio)
  {
    i32 L1size = std::floor(L1ratio * L2Usize);
    return (L1size % 2) ? L1size : L1size + 1;
  }

  static cv::Mat CalculateL1(const cv::Mat& L2U, const cv::Point2d& L2Upeak, i32 L1size) { return RoiCropRef(L2U, L2Upeak.x, L2Upeak.y, L1size, L1size); }

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

  static cv::Point2d GetPixelShift(const cv::Point2d& L3peak, const cv::Point2d& L3mid) { return L3peak - L3mid; }

  template <bool DebugMode>
  cv::Point2d GetSubpixelShift(const cv::Mat& L3, const cv::Point2d& L3peak, const cv::Point2d& L3mid, i32 L2size) const
  {
    PROFILE_FUNCTION;
    while (IsOutOfBounds(L3peak, L3, L2size))
      if (!ReduceL2size<DebugMode>(L2size))
        return L3peak - L3mid;

    cv::Mat L2 = CalculateL2<DebugMode>(L3, L3peak, L2size);
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
  static cv::Mat ColorComposition(const cv::Mat& img1, const cv::Mat& img2);
  static std::vector<cv::Mat> LoadImages(const std::string& imagesDirectory);
  std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>> CreateImagePairs(const std::vector<cv::Mat>& images, f64 maxShift, i32 itersPerImage, f64 noiseStdev) const;
  IterativePhaseCorrelation<Float> CreateIPCFromParams(const std::vector<f64>& params) const;
  std::function<f64(const std::vector<f64>&)> CreateObjectiveFunction(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs) const;
  std::function<f64(const std::vector<f64>&)> CreateObjectiveFunction(const std::function<f64(const IterativePhaseCorrelation&)>& obj) const;
  static std::vector<f64> CalculateOptimalParameters(const std::function<f64(const std::vector<f64>&)>& obj, const std::function<f64(const std::vector<f64>&)>& valid, i32 populationSize);
  void ApplyOptimalParameters(const std::vector<f64>& optimalParameters);
  static void ShowOptimizationPlots(const std::vector<cv::Point2d>& shiftsReference, const std::vector<cv::Point2d>& shiftsPixel, const std::vector<cv::Point2d>& shiftsNonit,
      const std::vector<cv::Point2d>& shiftsBefore, const std::vector<cv::Point2d>& shiftsAfter);
  std::vector<cv::Point2d> GetShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs) const;
  std::vector<cv::Point2d> GetNonIterativeShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs) const;
  std::vector<cv::Point2d> GetPixelShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs) const;
  static std::vector<cv::Point2d> GetReferenceShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs);
  static f64 GetAverageAccuracy(const std::vector<cv::Point2d>& shiftsReference, const std::vector<cv::Point2d>& shifts);
  static void ShowRandomImagePair(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2d>>& imagePairs);
};
