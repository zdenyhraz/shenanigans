#pragma once
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Fourier/fourier.h"
#include "Astrophysics/FITS.h"
#include "Filtering/filtering.h"
#include "Log/Logger.h"
#include "Log/LogFunction.h"
#include "Optimization/Evolution.h"
#include "Plot/Plot2D.h"
#include "Plot/Plot1D.h"

class IterativePhaseCorrelation
{
public:
  enum class BandpassType : u8
  {
    Rectangular,
    Gaussian,
  };

  enum class WindowType : u8
  {
    Rectangular,
    Hann,
  };

  enum class InterpolationType : u8
  {
    NearestNeighbor,
    Linear,
    Cubic,
  };

  enum class AccuracyType : u8
  {
    Pixel,
    Subpixel,
    SubpixelIterative
  };

  IterativePhaseCorrelation(i32 rows, i32 cols = 0, f64 bandpassL = 0, f64 bandpassH = 1) { Initialize(rows, cols, bandpassL, bandpassH); }
  IterativePhaseCorrelation(const cv::Size& size, f64 bandpassL = 0, f64 bandpassH = 1) { Initialize(size.height, size.width, bandpassL, bandpassH); }
  IterativePhaseCorrelation(const cv::Mat& img, f64 bandpassL = 0, f64 bandpassH = 1) { Initialize(img.rows, img.cols, bandpassL, bandpassH); }

  void Initialize(i32 rows, i32 cols, f64 bandpassL = 0, f64 bandpassH = 1)
  {
    SetSize(rows, cols);
    SetBandpassParameters(bandpassL, bandpassH);
    UpdateL1circle();
  }
  void SetSize(i32 rows, i32 cols = -1)
  {
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
    mBandpassL = clamp(bandpassL, -Constants::Inf, 1); // L from [-inf, 1]
    mBandpassH = clamp(bandpassH, 0, Constants::Inf);  // H from [0, inf]
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
  void SetWindowType(WindowType type) { mWindowType = type; }
  void SetDebugDirectory(const std::string& dir) const
  {
    mDebugDirectory = dir;
    if (not std::filesystem::exists(dir))
      std::filesystem::create_directory(dir);
  }
  void SetDebugName(const std::string& name) const { mDebugName = name; }

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

  template <bool DebugMode = false, bool CrossCorrelation = false>
  cv::Point2f Calculate(const cv::Mat& image1, const cv::Mat& image2) const
  {
    return Calculate<DebugMode, CrossCorrelation>(image1.clone(), image2.clone());
  }

  template <bool DebugMode = false, bool CrossCorrelation = false>
  cv::Point2f Calculate(cv::Mat&& image1, cv::Mat&& image2) const
  try
  {
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::Calculate");

    if (image1.size() != image2.size())
      throw std::runtime_error(fmt::format("Image sizes differ ({} != {})", image1.size(), image2.size()));

    if (image1.size() != cv::Size(mCols, mRows))
      throw std::runtime_error(fmt::format("Invalid image size ({} != {})", image1.size(), cv::Size(mCols, mRows)));

    if (image1.channels() != 1 or image2.channels() != 1)
      throw std::runtime_error("Multichannel images are not supported");

    ConvertToUnitFloat<DebugMode, CrossCorrelation>(image1);
    ConvertToUnitFloat<DebugMode, CrossCorrelation>(image2);

    if constexpr (DebugMode)
    {
      Plot2D::Set(fmt::format("{} I1", mDebugName));
      Plot2D::SetSavePath(fmt::format("{}/{}_I1.png", mDebugDirectory, mDebugName));
      Plot2D::SetColorMapType(QCPColorGradient::gpGrayscale);
      Plot2D::Plot(image1);

      Plot2D::Set(fmt::format("{} I2", mDebugName));
      Plot2D::SetSavePath(fmt::format("{}/{}_I2.png", mDebugDirectory, mDebugName));
      Plot2D::SetColorMapType(QCPColorGradient::gpGrayscale);
      Plot2D::Plot(image2);
    }

    ApplyWindow<DebugMode>(image1);
    ApplyWindow<DebugMode>(image2);

    auto dft1 = CalculateFourierTransform<DebugMode>(std::move(image1));
    auto dft2 = CalculateFourierTransform<DebugMode>(std::move(image2));

    if constexpr (DebugMode and 0)
    {
      auto plot1 = dft1.clone();
      Fourier::fftshift(plot1);

      Plot2D::Set(fmt::format("{} DFT1lm", mDebugName));
      Plot2D::SetSavePath(fmt::format("{}/{}_DFT1logmagn.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(Fourier::logmagn(plot1));

      Plot2D::Set(fmt::format("{} DFT1p", mDebugName));
      Plot2D::SetSavePath(fmt::format("{}/{}_DFT1phase.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(Fourier::phase(plot1));

      auto plot2 = dft2.clone();
      Fourier::fftshift(plot2);

      Plot2D::Set(fmt::format("{} DFT2lm", mDebugName));
      Plot2D::SetSavePath(fmt::format("{}/{}_DFT2logmagn.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(Fourier::logmagn(plot2));

      Plot2D::Set(fmt::format("{} DFT2p", mDebugName));
      Plot2D::SetSavePath(fmt::format("{}/{}_DFT2phase.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(Fourier::phase(plot2));
    }

    auto crosspower = CalculateCrossPowerSpectrum<DebugMode, CrossCorrelation>(std::move(dft1), std::move(dft2));

    if constexpr (DebugMode)
    {
      Plot2D::Set(fmt::format("{} CP log magnitude", mDebugName));
      Plot2D::SetSavePath(fmt::format("{}/{}_CPlogmagn.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(Fourier::fftshift(Fourier::logmagn(crosspower)));

      Plot2D::Set(fmt::format("{} CP phase", mDebugName));
      Plot2D::SetSavePath(fmt::format("{}/{}_CPphase.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(Fourier::fftshift(Fourier::phase(crosspower)));
    }

    cv::Mat L3 = CalculateL3<DebugMode>(std::move(crosspower));
    cv::Point2f L3peak = GetPeak<DebugMode>(L3);
    cv::Point2f L3mid(L3.cols / 2, L3.rows / 2);
    cv::Point2f result = L3peak - L3mid;

    if constexpr (DebugMode)
    {
      auto plot = L3.clone();
      // normalize(plot, plot, 0, 1, cv::NORM_MINMAX);
      Plot2D::Set(fmt::format("{} L3", mDebugName));
      Plot2D::SetSavePath(fmt::format("{}/{}_L3.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(plot);

      if (0) // gradual peakshift
      {
        auto peakshift = roicrop(L3, L3.cols / 2, L3.rows / 2, 5, 5);
        resize(peakshift, peakshift, cv::Size(512, 512), 0, 0, cv::INTER_CUBIC);
        Plot2D::Set(fmt::format("{} peakshift", mDebugName));
        Plot2D::SetSavePath(fmt::format("{}/{}_peakshift.png", mDebugDirectory, mDebugName));
        Plot2D::Plot(peakshift);
      }
    }

    if (mAccuracyType == AccuracyType::Pixel)
      return L3peak - L3mid;

    if (mAccuracyType == AccuracyType::Subpixel)
    {
      i32 L2size = 5;
      while (IsOutOfBounds(L3peak, L3, L2size))
        if (!ReduceL2size<DebugMode>(L2size))
          return result;

      cv::Mat L2 = CalculateL2<DebugMode>(L3, L3peak, 5);
      cv::Point2f L2peak = GetPeakSubpixel<false>(L2, cv::Mat());
      cv::Point2f L2mid(L2.cols / 2, L2.rows / 2);
      return L3peak - L3mid + L2peak - L2mid;
    }

    // reduce the L2size as long as the L2 is out of bounds, return pixel level estimation accuracy if it cannot be reduced anymore
    i32 L2size = mL2size;
    while (IsOutOfBounds(L3peak, L3, L2size))
      if (!ReduceL2size<DebugMode>(L2size))
        return result;

    // L2
    cv::Mat L2 = CalculateL2<DebugMode>(L3, L3peak, L2size);
    cv::Point2f L2mid(L2.cols / 2, L2.rows / 2);
    if constexpr (DebugMode)
    {
      auto plot = L2.clone();
      // normalize(plot, plot, 0, 1, cv::NORM_MINMAX);
      resize(plot, plot, plot.size() * mUpsampleCoeff, 0, 0, cv::INTER_NEAREST);
      Plot2D::Set(fmt::format("{} L2", mDebugName));
      Plot2D::SetSavePath(fmt::format("{}/{}_L2.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(plot);
    }

    // L2U
    cv::Mat L2U = CalculateL2U<DebugMode>(L2);
    cv::Point2f L2Umid(L2U.cols / 2, L2U.rows / 2);

    if constexpr (DebugMode)
    {
      auto plot = L2U.clone();
      // normalize(plot, plot, 0, 1, cv::NORM_MINMAX);
      Plot2D::Set(fmt::format("{} L2U", mDebugName));
      Plot2D::SetSavePath(fmt::format("{}/{}_L2U.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(plot);

      if (0)
      {
        cv::Mat nearest, linear, cubic;
        resize(L2, nearest, L2.size() * mUpsampleCoeff, 0, 0, cv::INTER_NEAREST);
        resize(L2, linear, L2.size() * mUpsampleCoeff, 0, 0, cv::INTER_LINEAR);
        resize(L2, cubic, L2.size() * mUpsampleCoeff, 0, 0, cv::INTER_CUBIC);

        // Plot2D::SetSavePath("IPCL2UN", mDebugDirectory + "/L2UN.png");
        Plot2D::Plot("IPCL2UN", nearest);
        // Plot2D::SetSavePath("IPCL2UL", mDebugDirectory + "/L2UL.png");
        Plot2D::Plot("IPCL2UL", linear);
        // Plot2D::SetSavePath("IPCL2UC", mDebugDirectory + "/L2UC.png");
        Plot2D::Plot("IPCL2UC", cubic);
      }
    }

    f64 L1ratio = mL1ratio;
    cv::Mat L1circle = mL1circle.clone();
    while (L1ratio > 0)
    {
      LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::IterativeRefinement");

      // reset L2U peak position
      cv::Point2f L2Upeak = L2Umid;

      // L1
      cv::Mat L1;
      i32 L1size = GetL1size(L2U.cols, L1ratio);
      cv::Point2f L1mid(L1size / 2, L1size / 2);
      cv::Point2f L1peak;
      if (L1circle.cols != L1size)
        L1circle = kirkl(L1size);

      if constexpr (DebugMode)
      {
        Plot2D::Set(fmt::format("{} L1B", mDebugName));
        Plot2D::SetSavePath(fmt::format("{}/{}_L1B.png", mDebugDirectory, mDebugName));
        Plot2D::Plot(CalculateL1(L2U, L2Upeak, L1size));
      }

      for (i32 iter = 0; iter < mMaxIterations; ++iter)
      {
        if (IsOutOfBounds(L2Upeak, L2U, L1size))
          break;

        L1 = CalculateL1(L2U, L2Upeak, L1size);
        L1peak = GetPeakSubpixel<true>(L1, L1circle);
        L2Upeak += cv::Point2f(std::round(L1peak.x - L1mid.x), std::round(L1peak.y - L1mid.y));

        if (AccuracyReached(L1peak, L1mid))
        {
          if constexpr (DebugMode)
          {
            Plot2D::Set(fmt::format("{} L1A", mDebugName));
            Plot2D::SetSavePath(fmt::format("{}/{}_L1A.png", mDebugDirectory, mDebugName));
            Plot2D::Plot(L1);
          }

          return L3peak - L3mid + (L2Upeak - L2Umid + L1peak - L1mid) / mUpsampleCoeff;
        }
      }

      // maximum iterations reached - reduce L1 size by reducing L1ratio
      ReduceL1ratio<DebugMode>(L1ratio);
    }

    throw std::runtime_error("L1 failed to converge with all L1ratios");
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("IPC Calculate error: {}", e.what());
    return {0, 0};
  }

  cv::Mat Align(const cv::Mat& image1, const cv::Mat& image2) const { return Align(image1.clone(), image2.clone()); }
  cv::Mat Align(cv::Mat&& image1, cv::Mat&& image2) const;
  std::tuple<cv::Mat, cv::Mat> CalculateFlow(const cv::Mat& image1, const cv::Mat& image2, f32 resolution) const { return CalculateFlow(image1.clone(), image2.clone(), resolution); }
  std::tuple<cv::Mat, cv::Mat> CalculateFlow(cv::Mat&& image1, cv::Mat&& image2, f32 resolution) const;
  void ShowDebugStuff() const;
  void Optimize(const std::string& trainingImagesDirectory, const std::string& validationImagesDirectory, f32 maxShift = 2.0, f32 noiseStdev = 0.01, i32 itersPerImage = 100, f64 validationRatio = 0.2,
      i32 populationSize = ParameterCount * 7, bool mute = false);
  void PlotObjectiveFunctionLandscape(const std::string& trainingImagesDirectory, f32 maxShift, f32 noiseStdev, i32 itersPerImage, i32 iters) const;
  void PlotImageSizeAccuracyDependence(const std::string& trainingImagesDirectory, f32 maxShift, f32 noiseStdev, i32 itersPerImage, i32 iters);
  void PlotUpsampleCoefficientAccuracyDependence(const std::string& trainingImagesDirectory, f32 maxShift, f32 noiseStdev, i32 itersPerImage, i32 iters) const;
  void PlotNoiseAccuracyDependence(const std::string& trainingImagesDirectory, f32 maxShift, f32 noiseStdev, i32 itersPerImage, i32 iters) const;
  void PlotNoiseOptimalBPHDependence(const std::string& trainingImagesDirectory, f32 maxShift, f32 noiseStdev, i32 itersPerImage, i32 iters) const;

private:
  i32 mRows = 0;
  i32 mCols = 0;
  f64 mBandpassL = 0;
  f64 mBandpassH = 1;
  i32 mL2size = 11;
  f64 mL1ratio = 0.35;
  f64 mL1ratioStep = 0.05;
  i32 mUpsampleCoeff = 51;
  i32 mMaxIterations = 20;
  BandpassType mBandpassType = BandpassType::Gaussian;
  InterpolationType mInterpolationType = InterpolationType::Linear;
  WindowType mWindowType = WindowType::Hann;
  mutable std::string mDebugDirectory = "Debug";
  mutable std::string mDebugName = "IPC";
  mutable AccuracyType mAccuracyType = AccuracyType::SubpixelIterative;
  cv::Mat mBandpass;
  cv::Mat mWindow;
  cv::Mat mL1circle;

  enum OptimizedParameters
  {
    BandpassTypeParameter,
    BandpassLParameter,
    BandpassHParameter,
    InterpolationTypeParameter,
    WindowTypeParameter,
    UpsampleCoeffParameter,
    L1ratioParameter,
    ParameterCount // last
  };

  void UpdateWindow()
  {
    switch (mWindowType)
    {
    case WindowType::Rectangular:
      mWindow = cv::Mat::ones(mRows, mCols, CV_32F);
      break;
    case WindowType::Hann:
      createHanningWindow(mWindow, cv::Size(mCols, mRows), CV_32F);
      break;
    }
  }

  void UpdateL1circle() { mL1circle = kirkl(GetL1size(mL2size * mUpsampleCoeff, mL1ratio)); }

  void UpdateBandpass()
  {
    mBandpass = cv::Mat::ones(mRows, mCols, CV_32F);

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
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::ConvertToUnitFloat");

    if (image.type() != CV_32F)
      image.convertTo(image, CV_32F);

    if constexpr (Normalize)
      normalize(image, image, 0, 1, cv::NORM_MINMAX);
  }
  template <bool DebugMode>
  void ApplyWindow(cv::Mat& image) const
  {
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::ApplyWindow");

    if (mWindowType != WindowType::Rectangular)
      multiply(image, mWindow, image);
  }
  template <bool DebugMode>
  static cv::Mat CalculateFourierTransform(cv::Mat&& image)
  {
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::CalculateFourierTransform");
    return Fourier::fft(std::move(image));
  }
  template <bool DebugMode, bool CrossCorrelation>
  cv::Mat CalculateCrossPowerSpectrum(cv::Mat&& dft1, cv::Mat&& dft2) const
  {
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
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::CalculateL3");
    cv::Mat L3 = Fourier::ifft(std::move(crosspower));
    Fourier::fftshift(L3);
    return L3;
  }
  template <bool DebugMode>
  static cv::Point2f GetPeak(const cv::Mat& mat)
  {
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::GetPeak");
    cv::Point2i peak;
    minMaxLoc(mat, nullptr, nullptr, nullptr, &peak);

    if (peak.x < 0 or peak.y < 0 or peak.x >= mat.cols or peak.y >= mat.rows)
      return cv::Point2f(0, 0);

    return peak;
  }
  template <bool Circular>
  cv::Point2f GetPeakSubpixel(const cv::Mat& mat, const cv::Mat& L1circle) const
  {
    f64 M = 0;
    f64 My = 0;
    f64 Mx = 0;

    if constexpr (Circular)
    {
      if (L1circle.size() != mat.size())
        throw std::runtime_error(fmt::format("L1circle size mismatch: {} != {}", L1circle.size(), mat.size()));

      for (i32 r = 0; r < mat.rows; ++r)
      {
        auto matp = mat.ptr<f32>(r);
        auto circp = L1circle.ptr<f32>(r);
        for (i32 c = 0; c < mat.cols; ++c)
        {
          M += matp[c] * circp[c];
          My += matp[c] * circp[c] * r;
          Mx += matp[c] * circp[c] * c;
        }
      }
    }
    else
    {
      for (i32 r = 0; r < mat.rows; ++r)
      {
        auto matp = mat.ptr<f32>(r);
        for (i32 c = 0; c < mat.cols; ++c)
        {
          M += matp[c];
          My += matp[c] * r;
          Mx += matp[c] * c;
        }
      }
    }

    cv::Point2f result(Mx / M, My / M);

    if (result.x < 0 or result.y < 0 or result.x >= mat.cols or result.y >= mat.rows)
      return cv::Point2f(mat.cols / 2, mat.rows / 2);

    return result;
  }
  template <bool DebugMode>
  static cv::Mat CalculateL2(const cv::Mat& L3, const cv::Point2f& L3peak, i32 L2size)
  {
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::CalculateL2");
    return roicrop(L3, L3peak.x, L3peak.y, L2size, L2size);
  }
  template <bool DebugMode>
  cv::Mat CalculateL2U(const cv::Mat& L2) const
  {
    LOG_FUNCTION_IF(DebugMode, "IterativePhaseCorrelation::CalculateL2U");

    cv::Mat L2U;
    switch (mInterpolationType)
    {
    case InterpolationType::NearestNeighbor:
      resize(L2, L2U, L2.size() * mUpsampleCoeff, 0, 0, cv::INTER_NEAREST);
      break;
    case InterpolationType::Linear:
      resize(L2, L2U, L2.size() * mUpsampleCoeff, 0, 0, cv::INTER_LINEAR);
      break;
    case InterpolationType::Cubic:
      resize(L2, L2U, L2.size() * mUpsampleCoeff, 0, 0, cv::INTER_CUBIC);
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
  static cv::Mat CalculateL1(const cv::Mat& L2U, const cv::Point2f& L2Upeak, i32 L1size) { return roicropref(L2U, L2Upeak.x, L2Upeak.y, L1size, L1size); }
  static bool IsOutOfBounds(const cv::Point2i& peak, const cv::Mat& mat, i32 size) { return IsOutOfBounds(peak, mat, {size, size}); }
  static bool IsOutOfBounds(const cv::Point2i& peak, const cv::Mat& mat, cv::Size size)
  {
    return peak.x - size.width / 2 < 0 or peak.y - size.height / 2 < 0 or peak.x + size.width / 2 >= mat.cols or peak.y + size.height / 2 >= mat.rows;
  }
  static bool AccuracyReached(const cv::Point2f& L1peak, const cv::Point2f& L1mid) { return abs(L1peak.x - L1mid.x) < 0.5 and abs(L1peak.y - L1mid.y) < 0.5; }
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
    L1ratio -= mL1ratioStep;
    if constexpr (DebugMode)
      LOG_WARNING("L1 did not converge - reducing L1ratio to {:.2f}", L1ratio);
  }
  static cv::Mat ColorComposition(const cv::Mat& img1, const cv::Mat& img2);
  static std::vector<cv::Mat> LoadImages(const std::string& imagesDirectory, bool mute = false);
  std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>> CreateImagePairs(const std::vector<cv::Mat>& images, f64 maxShift, i32 itersPerImage, f64 noiseStdev) const;
  static void AddNoise(cv::Mat& image, f64 noiseStdev);
  const std::function<f64(const std::vector<f64>&)> CreateObjectiveFunction(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs) const;
  static std::vector<f64> CalculateOptimalParameters(const std::function<f64(const std::vector<f64>&)>& obj, const std::function<f64(const std::vector<f64>&)>& valid, i32 populationSize, bool mute);
  void ApplyOptimalParameters(const std::vector<f64>& optimalParameters, bool mute);
  std::string BandpassType2String(BandpassType type, f64 bandpassL, f64 bandpassH) const;
  static std::string WindowType2String(WindowType type);
  static std::string InterpolationType2String(InterpolationType type);
  static void ShowOptimizationPlots(const std::vector<cv::Point2f>& shiftsReference, const std::vector<cv::Point2f>& shiftsPixel, const std::vector<cv::Point2f>& shiftsNonit,
      const std::vector<cv::Point2f>& shiftsBefore, const std::vector<cv::Point2f>& shiftsAfter);
  std::vector<cv::Point2f> GetShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs) const;
  std::vector<cv::Point2f> GetNonIterativeShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs) const;
  std::vector<cv::Point2f> GetPixelShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs) const;
  static std::vector<cv::Point2f> GetReferenceShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs);
  static f64 GetAverageAccuracy(const std::vector<cv::Point2f>& shiftsReference, const std::vector<cv::Point2f>& shifts);
  static void ShowRandomImagePair(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs);
  static f64 GetFractionalPart(f64 x);
};
