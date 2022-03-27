#pragma once
#include "Fourier/Fourier.hpp"
#include "IPCAlign.hpp"
#include "IPCDebug.hpp"
#include "IPCFlow.hpp"
#include "IPCMeasure.hpp"
#include "IPCOptimization.hpp"

class IPC
{
public:
  using Float = f64;
  friend class IPCAlign;
  friend class IPCDebug;
  friend class IPCFlow;
  friend class IPCMeasure;
  friend class IPCOptimization;

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

  enum class ModeType : u8
  {
    Debug,
    Release
  };

  struct Options
  {
    ModeType ModeT = ModeType::Release;
    AccuracyType AccuracyT = AccuracyType::SubpixelIterative;
  };

  explicit IPC(i32 rows, i32 cols = -1, f64 bpL = 0, f64 bpH = 1) { Initialize(rows, cols, bpL, bpH); }
  explicit IPC(const cv::Size& size, f64 bpL = 0, f64 bpH = 1) { Initialize(size.height, size.width, bpL, bpH); }
  explicit IPC(const cv::Mat& img, f64 bpL = 0, f64 bpH = 1) { Initialize(img.rows, img.cols, bpL, bpH); }

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

  void SetL2Usize(i32 L2Usize)
  {
    mL2Usize = std::max(L2Usize, mL2size);
    UpdateL1circle();
  }

  void SetCrossPowerEpsilon(f64 CPeps) { mCPeps = std::max(CPeps, 0.); }

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
  i32 GetL2Usize() const { return mL2Usize; }
  f64 GetCrossPowerEpsilon() const { return mCPeps; }
  cv::Mat GetWindow() const { return mWin; }
  cv::Mat GetBandpass() const { return mBP; }
  BandpassType GetBandpassType() const { return mBPT; }
  WindowType GetWindowType() const { return mWinT; }
  InterpolationType GetInterpolationType() const { return mIntT; }
  f64 GetUpsampleCoeff() const { return static_cast<Float>(mL2Usize) / mL2size; };
  f64 GetUpsampleCoeff(i32 L2size) const { return static_cast<Float>(mL2Usize) / L2size; };

  template <Options OptionsT = Options()>
  cv::Point2d Calculate(const cv::Mat& image1, const cv::Mat& image2) const
  {
    PROFILE_FUNCTION;
    return Calculate<OptionsT>(image1.clone(), image2.clone());
  }

  template <Options OptionsT = Options()>
  cv::Point2d Calculate(cv::Mat&& image1, cv::Mat&& image2) const
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION_IF(OptionsT.ModeT == ModeType::Debug, "IPC::Calculate");

    if (image1.size() != cv::Size(mCols, mRows))
      [[unlikely]] throw std::invalid_argument(fmt::format("Invalid image size ({} != {})", image1.size(), cv::Size(mCols, mRows)));

    if (image1.size() != image2.size())
      [[unlikely]] throw std::invalid_argument(fmt::format("Image sizes differ ({} != {})", image1.size(), image2.size()));

    if (image1.channels() != 1 or image2.channels() != 1)
      [[unlikely]] throw std::invalid_argument("Multichannel images are not supported");

    ConvertToUnitFloat(image1);
    ConvertToUnitFloat(image2);
    if constexpr (OptionsT.ModeT == ModeType::Debug)
      IPCDebug::DebugInputImages(*this, image1, image2);

    // windowing
    ApplyWindow(image1);
    ApplyWindow(image2);

    // ffts
    auto dft1 = CalculateFourierTransform(std::move(image1));
    auto dft2 = CalculateFourierTransform(std::move(image2));
    if constexpr (OptionsT.ModeT == ModeType::Debug and 0)
      IPCDebug::DebugFourierTransforms(*this, dft1, dft2);

    // cross-power
    auto crosspower = CalculateCrossPowerSpectrum<OptionsT>(std::move(dft1), std::move(dft2));
    if constexpr (OptionsT.ModeT == ModeType::Debug)
      IPCDebug::DebugCrossPowerSpectrum(*this, crosspower);

    // L3
    cv::Mat L3 = CalculateL3(std::move(crosspower));
    cv::Point2d L3peak = GetPeak(L3);
    cv::Point2d L3mid(L3.cols / 2, L3.rows / 2);
    if constexpr (OptionsT.ModeT == ModeType::Debug)
      IPCDebug::DebugL3(*this, L3);

    if constexpr (OptionsT.AccuracyT == AccuracyType::Pixel)
      return GetPixelShift(L3peak, L3mid);

    if constexpr (OptionsT.AccuracyT == AccuracyType::Subpixel)
      return GetSubpixelShift<OptionsT>(L3, L3peak, L3mid, 5);

    // reduce the L2size as long as the L2 is out of bounds, return pixel level estimation accuracy if it cannot be reduced anymore
    i32 L2size = mL2size;
    while (IsOutOfBounds(L3peak, L3, L2size))
      if (!ReduceL2size<OptionsT>(L2size))
        return L3peak - L3mid;

    // L2
    cv::Mat L2 = CalculateL2(L3, L3peak, L2size);
    cv::Point2d L2mid(L2.cols / 2, L2.rows / 2);
    if constexpr (OptionsT.ModeT == ModeType::Debug)
      IPCDebug::DebugL2(*this, L2);

    // L2U
    cv::Mat L2U = CalculateL2U(L2);
    cv::Point2d L2Umid(L2U.cols / 2, L2U.rows / 2);
    if constexpr (OptionsT.ModeT == ModeType::Debug)
      IPCDebug::DebugL2U(*this, L2, L2U);

    // L1
    i32 L1size;
    cv::Mat L1, L1circle;
    cv::Point2d L2Upeak, L1mid, L1peak;

    for (f64 L1ratio = mL1ratio; GetL1size(L2U.cols, L1ratio) > 0; L1ratio -= mL1ratioStep)
    {
      PROFILE_SCOPE(IterativeRefinement);
      if constexpr (OptionsT.ModeT == ModeType::Debug)
        LOG_DEBUG("Iterative refinement L1ratio: {:.2f}", L1ratio);

      L2Upeak = L2Umid; // reset L2U peak position
      L1size = GetL1size(L2U.cols, L1ratio);
      L1mid = cv::Point2d(L1size / 2, L1size / 2);
      L1circle = mL1circle.cols == L1size ? mL1circle : Kirkl<Float>(L1size);

      if constexpr (OptionsT.ModeT == ModeType::Debug and 0)
        IPCDebug::DebugL1B(*this, L2U, L1size, L3peak - L3mid, GetUpsampleCoeff(L2size));

      for (i32 iter = 0; iter < mMaxIter; ++iter)
      {
        PROFILE_SCOPE(IterativeRefinementIteration);
        if constexpr (OptionsT.ModeT == ModeType::Debug)
          LOG_DEBUG("Iterative refinement {} L2Upeak: {}", iter, L2Upeak);
        if (IsOutOfBounds(L2Upeak, L2U, L1size))
          [[unlikely]] break;

        L1 = CalculateL1(L2U, L2Upeak, L1size);
        if constexpr (OptionsT.ModeT == ModeType::Debug)
          IPCDebug::DebugL1A(*this, L1, L3peak - L3mid, L2Upeak - L2Umid, GetUpsampleCoeff(L2size));
        L1peak = GetPeakSubpixel<true>(L1, L1circle);
        L2Upeak += cv::Point2d(std::round(L1peak.x - L1mid.x), std::round(L1peak.y - L1mid.y));

        if (AccuracyReached(L1peak, L1mid))
          [[unlikely]]
          {
            if constexpr (OptionsT.ModeT == ModeType::Debug)
              IPCDebug::DebugL1A(*this, L1, L3peak - L3mid, L2Upeak - cv::Point2d(std::round(L1peak.x - L1mid.x), std::round(L1peak.y - L1mid.y)) - L2Umid, GetUpsampleCoeff(L2size), true);
            return L3peak - L3mid + (L2Upeak - L2Umid + L1peak - L1mid) / GetUpsampleCoeff(L2size);
          }
      }

      if constexpr (OptionsT.ModeT == ModeType::Debug)
        LOG_WARNING("L1 did not converge - reducing L1ratio: {:.2f} -> {:.2f}", L1ratio, L1ratio - mL1ratioStep);
    }

    if constexpr (OptionsT.ModeT == ModeType::Debug)
      LOG_WARNING("L1 failed to converge with all L1ratios, return non-iterative subpixel shift");
    return GetSubpixelShift<OptionsT>(L3, L3peak, L3mid, L2size);
  }

  static std::string BandpassType2String(BandpassType type);
  static std::string WindowType2String(WindowType type);
  static std::string InterpolationType2String(InterpolationType type);
  std::string Serialize() const;

private:
  i32 mRows = 0;
  i32 mCols = 0;
  f64 mBPL = 0;
  f64 mBPH = 1;
  i32 mL2size = 7;
  f64 mL1ratio = 0.45;
  f64 mL1ratioStep = 0.025;
  i32 mL1ratioStepCount = 4;
  i32 mL2Usize = 357;
  i32 mMaxIter = 10;
  f64 mCPeps = 0;
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
      mWin = cv::Mat();
      return;
    }
  }

  void UpdateL1circle() { mL1circle = Kirkl<Float>(GetL1size(mL2Usize, mL1ratio)); }

  void UpdateBandpass()
  {
    PROFILE_FUNCTION;
    mBP = cv::Mat::ones(mRows, mCols, GetMatType<Float>());

    if (mBPT == BandpassType::None)
      return;

    if (mBPL == 0 and mBPH == 0)
      return;

    if (mBPT == BandpassType::Rectangular and mBPL >= mBPH)
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
      else
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
      for (i32 r = 0; r < mRows; ++r)
      {
        auto bpp = mBP.ptr<Float>(r);
        for (i32 c = 0; c < mCols; ++c)
          bpp[c] = BandpassREquation(r, c);
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

  static void ConvertToUnitFloat(cv::Mat& image)
  {
    PROFILE_FUNCTION;
    image.convertTo(image, GetMatType<Float>());
  }

  void ApplyWindow(cv::Mat& image) const
  {
    PROFILE_FUNCTION;

    if (mWinT != WindowType::None)
      cv::multiply(image, mWin, image);
  }

  static cv::Mat CalculateFourierTransform(cv::Mat&& image)
  {
    PROFILE_FUNCTION;
    return Fourier::fft(std::move(image));
  }

  template <Options OptionsT>
  cv::Mat CalculateCrossPowerSpectrum(cv::Mat&& dft1, cv::Mat&& dft2) const
  {
    PROFILE_FUNCTION;
    const Float eps = mCPeps * dft1.rows * dft1.cols;
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

        dft1p[col][0] = re / (mag + eps) * band;
        dft1p[col][1] = im / (mag + eps) * band;
      }
    }
    return dft1;
  }

  static cv::Mat CalculateL3(cv::Mat&& crosspower)
  {
    PROFILE_FUNCTION;
    cv::Mat L3 = Fourier::ifft(std::move(crosspower));
    Fourier::fftshift(L3);
    return L3;
  }

  static cv::Point2d GetPeak(const cv::Mat& mat)
  {
    PROFILE_FUNCTION;
    cv::Point2i peak(0, 0);
    cv::minMaxLoc(mat, nullptr, nullptr, nullptr, &peak);
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

  static cv::Mat CalculateL2(const cv::Mat& L3, const cv::Point2d& L3peak, i32 L2size)
  {
    PROFILE_FUNCTION;
    return RoiCrop(L3, L3peak.x, L3peak.y, L2size, L2size);
  }

  cv::Mat CalculateL2U(const cv::Mat& L2) const
  {
    PROFILE_FUNCTION;

    cv::Mat L2U;
    switch (mIntT)
    {
    case InterpolationType::NearestNeighbor:
      cv::resize(L2, L2U, {mL2Usize, mL2Usize}, 0, 0, cv::INTER_NEAREST);
      break;
    case InterpolationType::Linear:
      cv::resize(L2, L2U, {mL2Usize, mL2Usize}, 0, 0, cv::INTER_LINEAR);
      break;
    case InterpolationType::Cubic:
      cv::resize(L2, L2U, {mL2Usize, mL2Usize}, 0, 0, cv::INTER_CUBIC);
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

  template <Options OptionsT>
  static bool ReduceL2size(i32& L2size)
  {
    L2size -= 2;
    if constexpr (OptionsT.ModeT == ModeType::Debug)
      LOG_WARNING("L2 out of bounds - reducing L2size to {}", L2size);
    return L2size >= 3;
  }

  static cv::Point2d GetPixelShift(const cv::Point2d& L3peak, const cv::Point2d& L3mid) { return L3peak - L3mid; }

  template <Options OptionsT>
  cv::Point2d GetSubpixelShift(const cv::Mat& L3, const cv::Point2d& L3peak, const cv::Point2d& L3mid, i32 L2size) const
  {
    PROFILE_FUNCTION;
    while (IsOutOfBounds(L3peak, L3, L2size))
      if (!ReduceL2size<OptionsT>(L2size))
        return L3peak - L3mid;

    cv::Mat L2 = CalculateL2(L3, L3peak, L2size);
    cv::Point2d L2peak = GetPeakSubpixel<false>(L2, cv::Mat());
    cv::Point2d L2mid(L2.cols / 2, L2.rows / 2);
    return L3peak - L3mid + L2peak - L2mid;
  }
};
