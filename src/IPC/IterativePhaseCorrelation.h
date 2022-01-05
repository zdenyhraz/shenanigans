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
  void SetL2size(i32 L2size) { mL2size = L2size % 2 ? L2size : L2size + 1; }
  void SetL1ratio(f64 L1ratio) { mL1ratio = L1ratio; }
  void SetUpsampleCoeff(i32 upsampleCoeff) { mUpsampleCoeff = upsampleCoeff % 2 ? upsampleCoeff : upsampleCoeff + 1; }
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
    if (image1.size() != image2.size())
      throw std::runtime_error(fmt::format("Image sizes differ ({} != {})", image1.size(), image2.size()));

    if (image1.size() != cv::Size(mCols, mRows))
      throw std::runtime_error(fmt::format("Invalid image size ({} != {})", image1.size(), cv::Size(mCols, mRows)));

    if (image1.channels() != 1 or image2.channels() != 1)
      throw std::runtime_error("Multichannel images are not supported");

    ConvertToUnitFloat(image1);
    ConvertToUnitFloat(image2);

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

    ApplyWindow(image1);
    ApplyWindow(image2);

    auto dft1 = CalculateFourierTransform(std::move(image1));
    auto dft2 = CalculateFourierTransform(std::move(image2));

    if constexpr (DebugMode and 0)
    {
      auto plot1 = dft1.clone();
      Fourier::fftshift(plot1);

      Plot2D::Set(fmt::format("{} DFT1lm", mDebugName));
      Plot2D::SetSavePath(fmt::format("{}/{}_DFT1lm.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(Fourier::logmagn(plot1));

      Plot2D::Set(fmt::format("{} DFT1p", mDebugName));
      Plot2D::SetSavePath(fmt::format("{}/{}_DFT1p.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(Fourier::phase(plot1));

      auto plot2 = dft2.clone();
      Fourier::fftshift(plot2);

      Plot2D::Set(fmt::format("{} DFT2lm", mDebugName));
      Plot2D::SetSavePath(fmt::format("{}/{}_DFT2lm.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(Fourier::logmagn(plot2));

      Plot2D::Set(fmt::format("{} DFT2p", mDebugName));
      Plot2D::SetSavePath(fmt::format("{}/{}_DFT2p.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(Fourier::phase(plot2));
    }

    auto crosspower = CalculateCrossPowerSpectrum<CrossCorrelation>(std::move(dft1), std::move(dft2));
    if constexpr (DebugMode)
    {
      Plot2D::Set(fmt::format("{} CP log magnitude", mDebugName));
      // Plot2D::SetSavePath(fmt::format("{}/{}_CPlogmagn.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(Fourier::fftshift(Fourier::logmagn(crosspower)));

      Plot2D::Set(fmt::format("{} CP phase", mDebugName));
      // Plot2D::SetSavePath(fmt::format("{}/{}_CPphase.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(Fourier::fftshift(Fourier::phase(crosspower)));
    }
    ApplyBandpass(crosspower);
    if constexpr (DebugMode)
    {
      Plot2D::Set(fmt::format("{} CP log magnitude filtered", mDebugName));
      // Plot2D::SetSavePath(fmt::format("{}/{}_CPlogmagn_filtered.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(Fourier::fftshift(Fourier::logmagn(crosspower)));

      Plot2D::Set(fmt::format("{} CP phase (fake) filtered", mDebugName));
      // Plot2D::SetSavePath(fmt::format("{}/{}_CPphase_filtered.png", mDebugDirectory, mDebugName));
      Plot2D::Plot(mBandpass.mul(Fourier::fftshift(Fourier::phase(crosspower))));
    }

    cv::Mat L3 = CalculateL3(std::move(crosspower));
    cv::Point2f L3peak = GetPeak(L3);
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

      cv::Mat L2 = CalculateL2(L3, L3peak, 5);
      cv::Point2f L2peak = GetPeakSubpixel(L2);
      cv::Point2f L2mid(L2.cols / 2, L2.rows / 2);
      return L3peak - L3mid + L2peak - L2mid;
    }

    i32 L2size = mL2size;
    f64 L1ratio = mL1ratio;

    // reduce the L2size as long as the L2 is out of bounds, return pixel level estimation accuracy if it cannot be reduced anymore
    while (IsOutOfBounds(L3peak, L3, L2size))
      if (!ReduceL2size<DebugMode>(L2size))
        return result;

    // L2
    cv::Mat L2 = CalculateL2(L3, L3peak, L2size);
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
    cv::Mat L2U = CalculateL2U(L2);
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

    while (L1ratio > 0)
    {
      // reset L2U peak position
      cv::Point2f L2Upeak = L2Umid;

      // L1
      cv::Mat L1;
      i32 L1size = GetL1size(L2U, L1ratio);
      cv::Point2f L1mid(L1size / 2, L1size / 2);
      cv::Point2f L1peak;
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
        L1peak = GetPeakSubpixel(L1);
        L2Upeak += cv::Point2f(round(L1peak.x - L1mid.x), round(L1peak.y - L1mid.y));

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
  cv::Mat mFrequencyBandpass;
  cv::Mat mWindow;

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

    CalculateFrequencyBandpass();
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
  void ConvertToUnitFloat(cv::Mat& image) const
  {
    if (image.type() != CV_32F)
      image.convertTo(image, CV_32F);
    normalize(image, image, 0, 1, cv::NORM_MINMAX);
  }
  void ApplyWindow(cv::Mat& image) const
  {
    if (mWindowType == WindowType::Rectangular)
      return;
    multiply(image, mWindow, image);
  }
  cv::Mat CalculateFourierTransform(cv::Mat&& image) const { return Fourier::fft(std::move(image)); }

  template <bool CrossCorrelation>
  cv::Mat CalculateCrossPowerSpectrum(cv::Mat&& dft1, cv::Mat&& dft2) const
  {
    for (i32 row = 0; row < dft1.rows; ++row)
    {
      auto dft1p = dft1.ptr<cv::Vec2f>(row);
      auto dft2p = dft2.ptr<cv::Vec2f>(row);
      for (i32 col = 0; col < dft1.cols; ++col)
      {
        const f32 re = dft1p[col][0] * dft2p[col][0] + dft1p[col][1] * dft2p[col][1];
        const f32 im = dft1p[col][0] * dft2p[col][1] - dft1p[col][1] * dft2p[col][0];
        const f32 mag = sqrt(re * re + im * im);

        if constexpr (CrossCorrelation)
        {
          // reuse dft1 memory
          dft1p[col][0] = re;
          dft1p[col][1] = im;
        }
        else
        {
          // reuse dft1 memory
          dft1p[col][0] = re / mag;
          dft1p[col][1] = im / mag;
        }
      }
    }
    return dft1;
  }
  void ApplyBandpass(cv::Mat& crosspower) const
  {
    if (mBandpassL <= 0 and mBandpassH >= 1)
      return;

    multiply(crosspower, mFrequencyBandpass, crosspower);
  }
  void CalculateFrequencyBandpass()
  {
    mFrequencyBandpass = Fourier::dupchansc(mBandpass);
    Fourier::ifftshift(mFrequencyBandpass);
  }
  cv::Mat CalculateL3(cv::Mat&& crosspower) const
  {
    cv::Mat L3 = Fourier::ifft(std::move(crosspower));
    Fourier::fftshift(L3);
    return L3;
  }
  cv::Point2f GetPeak(const cv::Mat& mat) const
  {
    cv::Point2i peak;
    minMaxLoc(mat, nullptr, nullptr, nullptr, &peak);

    if (peak.x < 0 or peak.y < 0 or peak.x >= mat.cols or peak.y >= mat.rows)
      return cv::Point2f(0, 0);

    return peak;
  }
  cv::Point2f GetPeakSubpixel(const cv::Mat& mat) const
  {
    f64 M = 0;
    f64 My = 0;
    f64 Mx = 0;

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

    cv::Point2f result(Mx / M, My / M);

    if (result.x < 0 or result.y < 0 or result.x >= mat.cols or result.y >= mat.rows)
      return cv::Point2f(mat.cols / 2, mat.rows / 2);

    return result;
  }
  cv::Mat CalculateL2(const cv::Mat& L3, const cv::Point2f& L3peak, i32 L2size) const { return roicrop(L3, L3peak.x, L3peak.y, L2size, L2size); }
  cv::Mat CalculateL2U(const cv::Mat& L2) const
  {
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
  i32 GetL1size(const cv::Mat& L2U, f64 L1ratio) const
  {
    i32 L1size = std::floor(L1ratio * L2U.cols);
    L1size = L1size % 2 ? L1size : L1size + 1;
    return L1size;
  }
  cv::Mat CalculateL1(const cv::Mat& L2U, const cv::Point2f& L2Upeak, i32 L1size) const { return kirklcrop(L2U, L2Upeak.x, L2Upeak.y, L1size); }
  bool IsOutOfBounds(const cv::Point2i& peak, const cv::Mat& mat, i32 size) const { return IsOutOfBounds(peak, mat, {size, size}); }
  bool IsOutOfBounds(const cv::Point2i& peak, const cv::Mat& mat, cv::Size size) const
  {
    return peak.x - size.width / 2 < 0 or peak.y - size.height / 2 < 0 or peak.x + size.width / 2 >= mat.cols or peak.y + size.height / 2 >= mat.rows;
  }
  bool AccuracyReached(const cv::Point2f& L1peak, const cv::Point2f& L1mid) const { return abs(L1peak.x - L1mid.x) < 0.5 and abs(L1peak.y - L1mid.y) < 0.5; }
  template <bool DebugMode>
  bool ReduceL2size(i32& L2size) const
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
  static cv::Mat ColorComposition(const cv::Mat& img1, const cv::Mat& img2)
  {
    const cv::Vec3f img1clr = {1, 0.5, 0};
    const cv::Vec3f img2clr = {0, 0.5, 1};

    const f32 gamma1 = 1.0;
    const f32 gamma2 = 1.0;

    cv::Mat img1c = cv::Mat::zeros(img1.size(), CV_32FC3);
    cv::Mat img2c = cv::Mat::zeros(img2.size(), CV_32FC3);

    for (i32 row = 0; row < img1.rows; ++row)
    {
      auto img1p = img1.ptr<f32>(row);
      auto img2p = img2.ptr<f32>(row);
      auto img1cp = img1c.ptr<cv::Vec3f>(row);
      auto img2cp = img2c.ptr<cv::Vec3f>(row);

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

  std::vector<cv::Mat> LoadImages(const std::string& imagesDirectory, bool mute = false) const
  {
    if (!mute)
      LOG_INFO("Loading images from '{}'...", imagesDirectory);

    if (!std::filesystem::is_directory(imagesDirectory))
      throw std::runtime_error(fmt::format("Directory '{}' is not a valid directory", imagesDirectory));

    std::vector<cv::Mat> images;
    for (const auto& entry : std::filesystem::directory_iterator(imagesDirectory))
    {
      const std::string path = entry.path().string();

      if (!IsImage(path))
      {
        if (!mute)
          LOG_DEBUG("Directory contains a non-image file {}", path);
        continue;
      }

      // crop the input image - good for solar images, omits the black borders
      static constexpr f32 cropFocusRatio = 0.5;
      auto image = loadImage(path);
      image = roicropmid(image, cropFocusRatio * image.cols, cropFocusRatio * image.rows);
      images.push_back(image);
      if (!mute)
        LOG_DEBUG("Loaded image {}", path);
    }
    return images;
  }

  std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>> CreateImagePairs(const std::vector<cv::Mat>& images, f64 maxShift, i32 itersPerImage, f64 noiseStdev) const
  {
    for (const auto& image : images)
    {
      if (image.rows < mRows + maxShift or image.cols < mCols + maxShift)
        throw std::runtime_error(fmt::format("Could not optimize IPC parameters - input image is too small for specified IPC window size & max shift ratio ([{},{}] < [{},{}])", image.rows, image.cols,
            mRows + maxShift, mCols + maxShift));
    }

    std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>> imagePairs;
    imagePairs.reserve(images.size() * itersPerImage);

    for (const auto& image : images)
    {
      for (i32 i = 0; i < itersPerImage; ++i)
      {
        // random shift from a random point
        cv::Point2f shift(rand11() * maxShift, rand11() * maxShift);
        cv::Point2i point(clamp(rand01() * image.cols, mCols, image.cols - mCols), clamp(rand01() * image.rows, mRows, image.rows - mRows));
        cv::Mat T = (cv::Mat_<f32>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
        cv::Mat imageShifted;
        warpAffine(image, imageShifted, T, image.size());
        cv::Mat image1 = roicrop(image, point.x, point.y, mCols, mRows);
        cv::Mat image2 = roicrop(imageShifted, point.x, point.y, mCols, mRows);

        ConvertToUnitFloat(image1);
        ConvertToUnitFloat(image2);

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
    return imagePairs;
  }
  void AddNoise(cv::Mat& image, f64 noiseStdev) const
  {
    if (noiseStdev <= 0)
      return;

    cv::Mat noise = cv::Mat::zeros(image.rows, image.cols, CV_32F);
    randn(noise, 0, noiseStdev);
    image += noise;
  }
  const std::function<f64(const std::vector<f64>&)> CreateObjectiveFunction(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs) const
  {
    return [&](const std::vector<f64>& params)
    {
      IterativePhaseCorrelation ipc(mRows, mCols);
      ipc.SetBandpassType(static_cast<BandpassType>((i32)params[BandpassTypeParameter]));
      ipc.SetBandpassParameters(params[BandpassLParameter], params[BandpassHParameter]);
      ipc.SetInterpolationType(static_cast<InterpolationType>((i32)params[InterpolationTypeParameter]));
      ipc.SetWindowType(static_cast<WindowType>((i32)params[WindowTypeParameter]));
      ipc.SetUpsampleCoeff(params[UpsampleCoeffParameter]);
      ipc.SetL1ratio(params[L1ratioParameter]);

      if (std::floor(ipc.GetL2size() * ipc.GetUpsampleCoeff() * ipc.GetL1ratio()) < 3)
        return std::numeric_limits<f64>::max();

      f64 avgerror = 0;
      for (const auto& [image1, image2, shift] : imagePairs)
      {
        const auto error = ipc.Calculate(image1, image2) - shift;
        avgerror += sqrt(error.x * error.x + error.y * error.y);
      }
      return imagePairs.size() > 0 ? avgerror / imagePairs.size() : 0.0f;
    };
  }
  std::vector<f64> CalculateOptimalParameters(const std::function<f64(const std::vector<f64>&)>& obj, const std::function<f64(const std::vector<f64>&)>& valid, i32 populationSize, bool mute) const
  {
    Evolution evo(ParameterCount);
    evo.mNP = populationSize;
    evo.mMutStrat = Evolution::RAND1;
    evo.SetParameterNames({"BPT", "BPL", "BPH", "ITPT", "WINT", "UC", "L1R"});
    evo.mLB = {0, -.5, 0.0, 0, 0, 11, 0.1};
    evo.mUB = {2, 1.0, 1.5, 3, 2, 51, 0.8};
    evo.SetPlotOutput(!mute);
    evo.SetConsoleOutput(!mute);
    return evo.Optimize(obj, valid).optimum;
  }
  void ApplyOptimalParameters(const std::vector<f64>& optimalParameters, bool mute)
  {
    if (optimalParameters.size() != ParameterCount)
      throw std::runtime_error("Cannot apply optimal parameters - wrong parameter count");

    SetBandpassType(static_cast<BandpassType>((i32)optimalParameters[BandpassTypeParameter]));
    SetBandpassParameters(optimalParameters[BandpassLParameter], optimalParameters[BandpassHParameter]);
    SetInterpolationType(static_cast<InterpolationType>((i32)optimalParameters[InterpolationTypeParameter]));
    SetWindowType(static_cast<WindowType>((i32)optimalParameters[WindowTypeParameter]));
    SetUpsampleCoeff(optimalParameters[UpsampleCoeffParameter]);
    SetL1ratio(optimalParameters[L1ratioParameter]);

    if (!mute)
    {
      LOG_INFO("Final IPC BandpassType: {}",
          BandpassType2String(static_cast<BandpassType>((i32)optimalParameters[BandpassTypeParameter]), optimalParameters[BandpassLParameter], optimalParameters[BandpassHParameter]));
      LOG_INFO("Final IPC BandpassL: {:.2f}", optimalParameters[BandpassLParameter]);
      LOG_INFO("Final IPC BandpassH: {:.2f}", optimalParameters[BandpassHParameter]);
      LOG_INFO("Final IPC InterpolationType: {}", InterpolationType2String(static_cast<InterpolationType>((i32)optimalParameters[InterpolationTypeParameter])));
      LOG_INFO("Final IPC WindowType: {}", WindowType2String(static_cast<WindowType>((i32)optimalParameters[WindowTypeParameter])));
      LOG_INFO("Final IPC UpsampleCoeff: {}", static_cast<i32>(optimalParameters[UpsampleCoeffParameter]));
      LOG_INFO("Final IPC L1ratio: {:.2f}", optimalParameters[L1ratioParameter]);
    }
  }
  std::string BandpassType2String(BandpassType type, f64 bandpassL, f64 bandpassH) const
  {
    switch (type)
    {
    case BandpassType::Rectangular:
      if (mBandpassL <= 0 and mBandpassH < 1)
        return "Rectangular low pass";
      else if (mBandpassL > 0 and mBandpassH >= 1)
        return "Rectangular high pass";
      else if (mBandpassL > 0 and mBandpassH < 1)
        return "Rectangular band pass";
      else if (mBandpassL <= 0 and mBandpassH >= 1)
        return "Rectangular all pass";
      else
        throw std::runtime_error("Unknown bandpass type");
    case BandpassType::Gaussian:
      if (mBandpassL <= 0 and mBandpassH < 1)
        return "Gaussian low pass";
      else if (mBandpassL > 0 and mBandpassH >= 1)
        return "Gaussian high pass";
      else if (mBandpassL > 0 and mBandpassH < 1)
        return "Gausian band pass";
      else if (mBandpassL <= 0 and mBandpassH >= 1)
        return "Gaussian all pass";
      else
        throw std::runtime_error("Unknown bandpass type");
    default:
      throw std::runtime_error("Unknown bandpass type");
    }
  }
  std::string WindowType2String(WindowType type) const
  {
    switch (type)
    {
    case WindowType::Rectangular:
      return "Rectangular";
    case WindowType::Hann:
      return "Hann";
    default:
      throw std::runtime_error("Unknown window type");
    }
  }
  std::string InterpolationType2String(InterpolationType type) const
  {
    switch (type)
    {
    case InterpolationType::NearestNeighbor:
      return "NearestNeighbor";
    case InterpolationType::Linear:
      return "Linear";
    case InterpolationType::Cubic:
      return "Cubic";
    default:
      throw std::runtime_error("Unknown interpolation type");
    }
  }
  void ShowOptimizationPlots(const std::vector<cv::Point2f>& shiftsReference, const std::vector<cv::Point2f>& shiftsPixel, const std::vector<cv::Point2f>& shiftsNonit,
      const std::vector<cv::Point2f>& shiftsBefore, const std::vector<cv::Point2f>& shiftsAfter) const
  {
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

    Plot1D::Set("IPCshift");
    Plot1D::SetXlabel("reference shift");
    Plot1D::SetYlabel("calculated shift");
    Plot1D::SetYnames({"reference", "pixel", "subpixel", "ipc", "ipc opt"});
    Plot1D::SetPens(
        {QPen(Plot::blue, Plot::pt / 2, Qt::DashLine), QPen(Plot::black, Plot::pt / 2, Qt::DashLine), QPen(Plot::orange, Plot::pt), QPen(Plot::magenta, Plot::pt), QPen(Plot::green, Plot::pt)});
    Plot1D::SetLegendPosition(Plot1D::LegendPosition::BotRight);
    Plot1D::Plot("IPCshift", shiftsXReference, {shiftsXReference, shiftsXPixel, shiftsXNonit, shiftsXBefore, shiftsXAfter});

    Plot1D::Set("IPCshifterror");
    Plot1D::SetXlabel("reference shift");
    Plot1D::SetYlabel("pixel error");
    Plot1D::SetYnames({"reference", "pixel", "subpixel", "ipc", "ipc opt"});
    Plot1D::SetPens(
        {QPen(Plot::blue, Plot::pt / 2, Qt::DashLine), QPen(Plot::black, Plot::pt / 2, Qt::DashLine), QPen(Plot::orange, Plot::pt), QPen(Plot::magenta, Plot::pt), QPen(Plot::green, Plot::pt)});
    Plot1D::SetLegendPosition(Plot1D::LegendPosition::BotRight);
    Plot1D::Plot("IPCshifterror", shiftsXReference, {shiftsXReferenceError, shiftsXPixelError, shiftsXNonitError, shiftsXBeforeError, shiftsXAfterError});
  }
  std::vector<cv::Point2f> GetShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs) const
  {
    std::vector<cv::Point2f> out;
    out.reserve(imagePairs.size());

    for (const auto& [image1, image2, referenceShift] : imagePairs)
      out.push_back(Calculate(image1, image2));

    return out;
  }
  std::vector<cv::Point2f> GetNonIterativeShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs) const
  {
    std::vector<cv::Point2f> out;
    out.reserve(imagePairs.size());

    mAccuracyType = AccuracyType::Subpixel;
    for (const auto& [image1, image2, referenceShift] : imagePairs)
      out.push_back(Calculate(image1, image2));
    mAccuracyType = AccuracyType::SubpixelIterative;

    return out;
  }
  std::vector<cv::Point2f> GetPixelShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs) const
  {
    std::vector<cv::Point2f> out;
    out.reserve(imagePairs.size());

    mAccuracyType = AccuracyType::Pixel;
    for (const auto& [image1, image2, referenceShift] : imagePairs)
      out.push_back(Calculate(image1, image2));
    mAccuracyType = AccuracyType::SubpixelIterative;

    return out;
  }
  std::vector<cv::Point2f> GetReferenceShifts(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs) const
  {
    std::vector<cv::Point2f> out;
    out.reserve(imagePairs.size());

    for (const auto& [image1, image2, referenceShift] : imagePairs)
      out.push_back(referenceShift);

    return out;
  }
  f64 GetAverageAccuracy(const std::vector<cv::Point2f>& shiftsReference, const std::vector<cv::Point2f>& shifts) const
  {
    if (shiftsReference.size() != shifts.size())
      throw std::runtime_error("Reference shift vector has different size than calculated shift vector");

    f64 avgerror = 0;
    for (usize i = 0; i < shifts.size(); ++i)
    {
      const auto error = shifts[i] - shiftsReference[i];
      avgerror += sqrt(error.x * error.x + error.y * error.y);
    }
    return avgerror / shifts.size();
  }
  void ShowRandomImagePair(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs)
  {
    const auto& [img1, img2, shift] = imagePairs[static_cast<usize>(rand01() * imagePairs.size())];
    cv::Mat concat;
    hconcat(img1, img2, concat);
    Plot2D::Set("Random image pair");
    Plot2D::SetColorMapType(QCPColorGradient::gpGrayscale);
    Plot2D::Plot("Random image pair", concat);
  }
  static f64 GetFractionalPart(f64 x) { return abs(x - std::floor(x)); }
};
