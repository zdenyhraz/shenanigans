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

template <bool DebugMode = false, bool CrossCorrelation = false, bool CudaFFT = false, bool PackedFFT = false>
class IterativePhaseCorrelation
{
public:
  enum class BandpassType
  {
    Rectangular,
    Gaussian,
  };

  enum class WindowType
  {
    Rectangular,
    Hann,
  };

  enum class InterpolationType
  {
    NearestNeighbor,
    Linear,
    Cubic,
  };

  enum class AccuracyType
  {
    Pixel,
    Subpixel,
    SubpixelIterative
  };

  IterativePhaseCorrelation(int rows, int cols = 0, double bandpassL = 0, double bandpassH = 1)
  {
    SetSize(rows, cols);
    SetBandpassParameters(bandpassL, bandpassH);
  }
  IterativePhaseCorrelation(const cv::Size& size, double bandpassL = 0, double bandpassH = 1)
  {
    SetSize(size);
    SetBandpassParameters(bandpassL, bandpassH);
  }
  IterativePhaseCorrelation(const cv::Mat& img, double bandpassL = 0, double bandpassH = 1)
  {
    SetSize(img.size());
    SetBandpassParameters(bandpassL, bandpassH);
  }

  void SetSize(int rows, int cols = -1)
  {
    mRows = rows;
    mCols = cols > 0 ? cols : rows;

    if (mWindow.rows != mRows || mWindow.cols != mCols)
      UpdateWindow();

    if (mBandpass.rows != mRows || mBandpass.cols != mCols)
      UpdateBandpass();
  }
  void SetSize(cv::Size size) { SetSize(size.height, size.width); }
  void SetBandpassParameters(double bandpassL, double bandpassH)
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
  void SetL2size(int L2size) { mL2size = L2size % 2 ? L2size : L2size + 1; }
  void SetL1ratio(double L1ratio) { mL1ratio = L1ratio; }
  void SetUpsampleCoeff(int upsampleCoeff) { mUpsampleCoeff = upsampleCoeff % 2 ? upsampleCoeff : upsampleCoeff + 1; }
  void SetMaxIterations(int maxIterations) { mMaxIterations = maxIterations; }
  void SetInterpolationType(InterpolationType interpolationType) { mInterpolationType = interpolationType; }
  void SetWindowType(WindowType type) { mWindowType = type; }
  void SetDebugDirectory(const std::string& dir) { mDebugDirectory = dir; }
  void SetDebugName(const std::string& name) { mDebugName = name; }

  int GetRows() const { return mRows; }
  int GetCols() const { return mCols; }
  int GetSize() const { return mRows; }
  double GetBandpassL() const { return mBandpassL; }
  double GetBandpassH() const { return mBandpassH; }
  int GetL2size() const { return mL2size; }
  double GetL1ratio() const { return mL1ratio; }
  int GetUpsampleCoeff() const { return mUpsampleCoeff; }
  cv::Mat GetWindow() const { return mWindow; }
  cv::Mat GetBandpass() const { return mBandpass; }

  cv::Point2f Calculate(const cv::Mat& image1, const cv::Mat& image2) const { return Calculate(image1.clone(), image2.clone()); }
  cv::Point2f Calculate(cv::Mat&& image1, cv::Mat&& image2) const
  try
  {
    if (image1.size() != image2.size())
      throw std::runtime_error(fmt::format("Image sizes differ ({} != {})", image1.size(), image2.size()));

    if (image1.size() != cv::Size(mCols, mRows))
      throw std::runtime_error(fmt::format("Invalid image size ({} != {})", image1.size(), cv::Size(mCols, mRows)));

    if (image1.channels() != 1 || image2.channels() != 1)
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

    if constexpr (DebugMode && 0)
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

    auto crosspower = CalculateCrossPowerSpectrum(std::move(dft1), std::move(dft2));
    ApplyBandpass(crosspower);

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
    }

    if (mAccuracyType == AccuracyType::Pixel)
      return L3peak - L3mid;

    if (mAccuracyType == AccuracyType::Subpixel)
    {
      int L2size = 5;
      while (IsOutOfBounds(L3peak, L3, L2size))
        if (!ReduceL2size(L2size))
          return result;

      cv::Mat L2 = CalculateL2(L3, L3peak, 5);
      cv::Point2f L2peak = GetPeakSubpixel(L2);
      cv::Point2f L2mid(L2.cols / 2, L2.rows / 2);
      return L3peak - L3mid + L2peak - L2mid;
    }

    int L2size = mL2size;
    double L1ratio = mL1ratio;

    // reduce the L2size as long as the L2 is out of bounds, return pixel level estimation accuracy if it cannot be reduced anymore
    while (IsOutOfBounds(L3peak, L3, L2size))
      if (!ReduceL2size(L2size))
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
      int L1size = GetL1size(L2U, L1ratio);
      cv::Point2f L1mid(L1size / 2, L1size / 2);
      cv::Point2f L1peak;
      if constexpr (DebugMode)
      {
        Plot2D::Set(fmt::format("{} L1B", mDebugName));
        Plot2D::SetSavePath(fmt::format("{}/{}_L1B.png", mDebugDirectory, mDebugName));
        Plot2D::Plot(CalculateL1(L2U, L2Upeak, L1size));
      }

      for (int iter = 0; iter < mMaxIterations; ++iter)
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
      ReduceL1ratio(L1ratio);
    }

    throw std::runtime_error("L1 failed to converge with all L1ratios");
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("IPC Calculate error: {}", e.what());
    return {0, 0};
  }

  cv::Mat Align(const cv::Mat& image1, const cv::Mat& image2) const { return Align(image1.clone(), image2.clone()); }
  cv::Mat Align(cv::Mat&& image1, cv::Mat&& image2) const
  try
  {
    cv::Mat img1W = image1.clone();
    cv::Mat img2W = image2.clone();
    ApplyWindow(img1W);
    ApplyWindow(img2W);
    cv::Mat img1FT = Fourier::fft(img1W, PackedFFT);
    cv::Mat img2FT = Fourier::fft(img2W, PackedFFT);
    Fourier::fftshift(img1FT);
    Fourier::fftshift(img2FT);
    cv::Mat img1FTm = cv::Mat::zeros(img1FT.size(), CV_32F);
    cv::Mat img2FTm = cv::Mat::zeros(img2FT.size(), CV_32F);
    for (int row = 0; row < img1FT.rows; ++row)
    {
      auto img1FTp = img1FT.ptr<cv::Vec2f>(row);
      auto img2FTp = img2FT.ptr<cv::Vec2f>(row);
      auto img1FTmp = img1FTm.ptr<float>(row);
      auto img2FTmp = img2FTm.ptr<float>(row);
      for (int col = 0; col < img1FT.cols; ++col)
      {
        const float& re1 = img1FTp[col][0];
        const float& im1 = img1FTp[col][1];
        const float& re2 = img2FTp[col][0];
        const float& im2 = img2FTp[col][1];
        img1FTmp[col] = log(sqrt(re1 * re1 + im1 * im1));
        img2FTmp[col] = log(sqrt(re2 * re2 + im2 * im2));
      }
    }
    cv::Point2f center((float)image1.cols / 2, (float)image1.rows / 2);
    double maxRadius = std::min(center.y, center.x);
    warpPolar(img1FTm, img1FTm, img1FTm.size(), center, maxRadius, cv::INTER_LINEAR | cv::WARP_FILL_OUTLIERS | cv::WARP_POLAR_LOG); // semilog Polar
    warpPolar(img2FTm, img2FTm, img2FTm.size(), center, maxRadius, cv::INTER_LINEAR | cv::WARP_FILL_OUTLIERS | cv::WARP_POLAR_LOG); // semilog Polar

    // rotation
    auto shiftR = Calculate(img1FTm, img2FTm);
    double rotation = -shiftR.y / image1.rows * 360;
    double scale = exp(shiftR.x * log(maxRadius) / image1.cols);
    Rotate(image2, -rotation, scale);

    // translation
    auto shiftT = Calculate(image1, image2);
    Shift(image2, -shiftT);

    if constexpr (DebugMode)
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

  std::tuple<cv::Mat, cv::Mat> CalculateFlow(const cv::Mat& image1, const cv::Mat& image2, float resolution) const { return CalculateFlow(image1.clone(), image2.clone(), resolution); }
  std::tuple<cv::Mat, cv::Mat> CalculateFlow(cv::Mat&& image1, cv::Mat&& image2, float resolution) const
  try
  {
    if (image1.size() != image2.size())
      throw std::runtime_error(fmt::format("Image sizes differ ({} != {})", image1.size(), image2.size()));

    if (mRows > image1.rows || mCols > image1.cols)
      throw std::runtime_error(fmt::format("Images are too small ({} < {})", image1.size(), cv::Size(mCols, mRows)));

    cv::Mat flowX = cv::Mat::zeros(cv::Size(resolution * image1.cols, resolution * image1.rows), CV_32F);
    cv::Mat flowY = cv::Mat::zeros(cv::Size(resolution * image2.cols, resolution * image2.rows), CV_32F);
    std::atomic<int> progress = 0;

#pragma omp parallel for
    for (int r = 0; r < flowX.rows; ++r)
    {
      if (++progress % (flowX.rows / 20) == 0)
        LOG_DEBUG("Calculating IPC flow profile ({:.0f}%)", static_cast<float>(progress) / flowX.rows * 100);

      for (int c = 0; c < flowX.cols; ++c)
      {
        const cv::Point2i center(c / resolution, r / resolution);

        if (IsOutOfBounds(center, image1, {mCols, mRows}))
          continue;

        const auto shift = Calculate(roicrop(image1, center.x, center.y, mCols, mRows), roicrop(image2, center.x, center.y, mCols, mRows));
        flowX.at<float>(r, c) = shift.x;
        flowY.at<float>(r, c) = shift.y;
      }
    }

    return {flowX, flowY};
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("IPC CalculateFlow error: {}", e.what());
    return {};
  }

  void ShowDebugStuff() const
  {
    // window
    if (1)
    {
      cv::Mat img = roicrop(loadImage("Resources/test.png"), 2048, 2048, mCols, mRows);
      cv::Mat w, imgw;
      createHanningWindow(w, img.size(), CV_32F);
      multiply(img, w, imgw);
      cv::Mat w0 = w.clone();
      cv::Mat r0 = cv::Mat::ones(w.size(), CV_32F);
      copyMakeBorder(w0, w0, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, cv::BORDER_CONSTANT, cv::Scalar::all(0));
      copyMakeBorder(r0, r0, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, cv::BORDER_CONSTANT, cv::Scalar::all(0));

      // Plot1D::Plot(GetIota(w0.cols, 1), {GetMidRow(r0), GetMidRow(w0)}, "1DWindows", "x", "window", {"cv::Rect", "Hann"}, Plot::pens, mDebugDirectory + "/1DWindows.png");
      // Plot1D::Plot(GetIota(w0.cols, 1), {GetMidRow(Fourier::fftlogmagn(r0)), GetMidRow(Fourier::fftlogmagn(w0))}, "1DWindowsDFT", "fx", "log DFT", {"cv::Rect", "Hann"}, Plot::pens, mDebugDirectory +
      // "/1DWindowsDFT.png");

      // Plot2D::SetSavePath("IPCdebug2D", mDebugDirectory + "/2DImage.png");
      Plot2D::Plot("IPCdebug2D", img);

      // Plot2D::SetSavePath("IPCdebug2D", mDebugDirectory + "/2DImageWindow.png");
      Plot2D::Plot("IPCdebug2D", imgw);

      // Plot2D::Plot(Fourier::fftlogmagn(r0), "2DWindowDFTR", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DWindowDFTR.png");
      // Plot2D::Plot(Fourier::fftlogmagn(w0), "2DWindowDFTH", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DWindowDFTH.png");
      // Plot2D::Plot(w, "2DWindow", "x", "y", "window", 0, 1, 0, 1, 0, mDebugDirectory + "/2DWindow.png");
      // Plot2D::Plot(Fourier::fftlogmagn(img), "2DImageDFT", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DImageDFT.png");
      // Plot2D::Plot(Fourier::fftlogmagn(imgw), "2DImageWindowDFT", "fx", "fy", "log DFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DImageWindowDFT.png");
    }

    // bandpass
    if (0)
    {
      cv::Mat bpR = cv::Mat::zeros(mRows, mCols, CV_32F);
      cv::Mat bpG = cv::Mat::zeros(mRows, mCols, CV_32F);
      for (int r = 0; r < mRows; ++r)
      {
        for (int c = 0; c < mCols; ++c)
        {
          bpR.at<float>(r, c) = BandpassREquation(r, c);

          if (mBandpassL <= 0 && mBandpassH < 1)
            bpG.at<float>(r, c) = LowpassEquation(r, c);
          else if (mBandpassL > 0 && mBandpassH >= 1)
            bpG.at<float>(r, c) = HighpassEquation(r, c);
          else if (mBandpassL > 0 && mBandpassH < 1)
            bpG.at<float>(r, c) = BandpassGEquation(r, c);
        }
      }

      if (mBandpassL > 0 && mBandpassH < 1)
        normalize(bpG, bpG, 0.0, 1.0, cv::NORM_MINMAX);

      cv::Mat bpR0, bpG0;
      copyMakeBorder(bpR, bpR0, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, cv::BORDER_CONSTANT, cv::Scalar::all(0));
      copyMakeBorder(bpG, bpG0, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, mUpsampleCoeff, cv::BORDER_CONSTANT, cv::Scalar::all(0));

      // Plot1D::Plot(GetIota(bpR0.cols, 1), {GetMidRow(bpR0), GetMidRow(bpG0)}, "b0", "x", "filter", {"cv::Rect", "Gauss"}, Plot::pens, mDebugDirectory + "/1DBandpass.png");
      // Plot1D::Plot(GetIota(bpR0.cols, 1), {GetMidRow(Fourier::ifftlogmagn(bpR0)), GetMidRow(Fourier::ifftlogmagn(bpG0))}, "b1", "fx", "log IDFT", {"cv::Rect", "Gauss"}, Plot::pens, mDebugDirectory +
      // "/1DBandpassIDFT.png"); Plot2D::Plot(bpR, "b2", "x", "y", "filter", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassR.png"); Plot2D::Plot(bpG, "b3", "x", "y", "filter", 0, 1, 0, 1, 0,
      // mDebugDirectory + "/2DBandpassG.png"); Plot2D::Plot(Fourier::ifftlogmagn(bpR0, 10), "b4", "fx", "fy", "log IDFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassRIDFT.png");
      // Plot2D::Plot(Fourier::ifftlogmagn(bpG0, 10), "b5", "fx", "fy", "log IDFT", 0, 1, 0, 1, 0, mDebugDirectory + "/2DBandpassGIDFT.png");
    }

    // bandpass image ringing
    if (0)
    {
      cv::Mat img = roicrop(loadImage("Resources/test.png"), 4098 / 2, 4098 / 2, mCols, mRows);
      cv::Mat fftR = Fourier::fft(img);
      cv::Mat fftG = Fourier::fft(img);
      cv::Mat filterR = cv::Mat::zeros(img.size(), CV_32F);
      cv::Mat filterG = cv::Mat::zeros(img.size(), CV_32F);

      for (int r = 0; r < mRows; ++r)
      {
        for (int c = 0; c < mCols; ++c)
        {
          filterR.at<float>(r, c) = BandpassREquation(r, c);

          if (mBandpassL <= 0 && mBandpassH < 1)
            filterG.at<float>(r, c) = LowpassEquation(r, c);
          else if (mBandpassL > 0 && mBandpassH >= 1)
            filterG.at<float>(r, c) = HighpassEquation(r, c);
          else if (mBandpassL > 0 && mBandpassH < 1)
            filterG.at<float>(r, c) = BandpassGEquation(r, c);
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

      // Plot2D::SetSavePath("IPCdebug2D", mDebugDirectory + "/2DBandpassImageR.png");
      Plot2D::Plot("IPCdebug2D", imgfR);

      // Plot2D::SetSavePath("IPCdebug2D", mDebugDirectory + "/2DBandpassImageG.png");
      Plot2D::Plot("IPCdebug2D", imgfG);
    }

    // 2 pic
    if (0)
    {
      cv::Point2f rawshift(rand11() * 0.25 * mCols, rand11() * 0.25 * mRows);
      cv::Mat image1 = roicrop(loadImage("Resources/test.png"), 4096 / 2, 4096 / 2, mCols, mRows);
      cv::Mat image2 = roicrop(loadImage("Resources/test.png"), 4096 / 2 - rawshift.x, 4096 / 2 - rawshift.y, mCols, mRows);

      if (1)
      {
        double noiseStdev = 0.03;
        cv::Mat noise1 = cv::Mat::zeros(image1.rows, image1.cols, CV_32F);
        cv::Mat noise2 = cv::Mat::zeros(image2.rows, image2.cols, CV_32F);
        randn(noise1, 0, noiseStdev);
        randn(noise2, 0, noiseStdev);
        image1 += noise1;
        image2 += noise2;
      }

      auto ipcshift = Calculate(image1, image2);

      LOG_INFO("Input raw shift = {}", rawshift);
      LOG_INFO("Resulting shift = {}", ipcshift);
      LOG_INFO("Resulting accuracy = {}", ipcshift - rawshift);
    }

    LOG_INFO("IPC debug stuff shown");
  }

  void Optimize(const std::string& trainingImagesDirectory, const std::string& validationImagesDirectory, float maxShift = 2.0, float noiseStdev = 0.01, int itersPerImage = 100,
      double validationRatio = 0.2, int populationSize = ParameterCount * 7, bool mute = false)
  try
  {
    if (!mute)
      LOG_FUNCTION("IPC optimization");

    if (itersPerImage < 1)
      throw std::runtime_error(fmt::format("Invalid iters per image ({})", itersPerImage));
    if (maxShift <= 0)
      throw std::runtime_error(fmt::format("Invalid max shift ({})", maxShift));
    if (noiseStdev < 0)
      throw std::runtime_error(fmt::format("Invalid noise stdev ({})", noiseStdev));

    const auto trainingImages = LoadImages(trainingImagesDirectory, mute);
    const auto validationImages = LoadImages(validationImagesDirectory, mute);

    if (trainingImages.empty())
      throw std::runtime_error("Empty training images vector");

    const auto trainingImagePairs = CreateImagePairs(trainingImages, maxShift, itersPerImage, noiseStdev);
    const auto validationImagePairs = CreateImagePairs(validationImages, maxShift, validationRatio * itersPerImage, noiseStdev);

    std::vector<cv::Point2f> referenceShifts, shiftsPixel, shiftsNonit, shiftsBefore;
    double objBefore;

    // before
    if (!mute)
    {
      LOG_INFO(
          "Running Iterative Phase Correlation parameter optimization on a set of {}/{} training/validation images with {}/{} image pairs - each generation, {} {}x{} IPCshifts will be calculated",
          trainingImages.size(), validationImages.size(), trainingImagePairs.size(), validationImagePairs.size(), populationSize * trainingImagePairs.size() + validationImagePairs.size(), mCols,
          mRows);
      ShowRandomImagePair(trainingImagePairs);
      referenceShifts = GetReferenceShifts(trainingImagePairs);
      shiftsPixel = GetPixelShifts(trainingImagePairs);
      shiftsNonit = GetNonIterativeShifts(trainingImagePairs);
      shiftsBefore = GetShifts(trainingImagePairs);
      objBefore = GetAverageAccuracy(referenceShifts, shiftsBefore);
      ShowOptimizationPlots(referenceShifts, shiftsPixel, shiftsNonit, shiftsBefore, {});
    }

    // opt
    const auto obj = CreateObjectiveFunction(trainingImagePairs);
    const auto valid = CreateObjectiveFunction(validationImagePairs);
    const auto optimalParameters = CalculateOptimalParameters(obj, valid, populationSize, mute);
    if (optimalParameters.empty())
      throw std::runtime_error("Optimization failed");
    ApplyOptimalParameters(optimalParameters, mute);

    // after
    if (!mute)
    {
      const auto shiftsAfter = GetShifts(trainingImagePairs);
      const auto objAfter = GetAverageAccuracy(referenceShifts, shiftsAfter);
      ShowOptimizationPlots(referenceShifts, shiftsPixel, shiftsNonit, shiftsBefore, shiftsAfter);
      LOG_INFO("Average pixel accuracy improvement: {:.3f} -> {:.3f} ({}%)", objBefore, objAfter, static_cast<int>((objBefore - objAfter) / objBefore * 100));
      LOG_SUCCESS("Iterative Phase Correlation parameter optimization successful");
    }
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("An error occured during Iterative Phase Correlation parameter optimization: {}", e.what());
  }
  void PlotObjectiveFunctionLandscape(const std::string& trainingImagesDirectory, float maxShift, float noiseStdev, int itersPerImage, int iters) const
  {
    LOG_FUNCTION("PlotObjectiveFunctionLandscape");
    const auto trainingImages = LoadImages(trainingImagesDirectory);
    const auto trainingImagePairs = CreateImagePairs(trainingImages, maxShift, itersPerImage, noiseStdev);
    const auto obj = CreateObjectiveFunction(trainingImagePairs);
    const int rows = iters;
    const int cols = iters;
    cv::Mat landscape(rows, cols, CV_32F);
    const double xmin = -0.25;
    const double xmax = 0.75;
    const double ymin = 0.25;
    const double ymax = 1.25;
    std::atomic<int> progress = 0;

#pragma omp parallel for
    for (int r = 0; r < rows; ++r)
    {
      for (int c = 0; c < cols; ++c)
      {
        LOG_INFO("Calculating objective function landscape ({:.1f}%)", (float)progress / (rows * cols - 1) * 100);
        std::vector<double> parameters(ParameterCount);

        // default
        parameters[BandpassTypeParameter] = static_cast<int>(mBandpassType);
        parameters[BandpassLParameter] = mBandpassL;
        parameters[BandpassHParameter] = mBandpassH;
        parameters[InterpolationTypeParameter] = static_cast<int>(mInterpolationType);
        parameters[WindowTypeParameter] = static_cast<int>(mWindowType);
        parameters[UpsampleCoeffParameter] = mUpsampleCoeff;
        parameters[L1ratioParameter] = mL1ratio;

        // modified
        parameters[BandpassLParameter] = xmin + (double)c / (cols - 1) * (xmax - xmin);
        parameters[BandpassHParameter] = ymin + (double)r / (rows - 1) * (ymax - ymin);

        landscape.at<float>(r, c) = std::log(obj(parameters));
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
  void PlotImageSizeAccuracyDependence(const std::string& trainingImagesDirectory, float maxShift, float noiseStdev, int itersPerImage, int iters)
  {
    const auto trainingImages = LoadImages(trainingImagesDirectory);
    std::vector<double> imageSizes(iters);
    std::vector<double> accuracy(iters);
    const double xmin = 16;
    const double xmax = mRows;
    std::atomic<int> progress = 0;

    for (int i = 0; i < iters; ++i)
    {
      LOG_INFO("Calculating image size accuracy dependence ({:.1f}%)", (float)progress / (iters - 1) * 100);
      int imageSize = xmin + (double)i / (iters - 1) * (xmax - xmin);
      imageSize = imageSize % 2 ? imageSize + 1 : imageSize;
      SetSize(imageSize, imageSize);
      const auto trainingImagePairs = CreateImagePairs(trainingImages, maxShift, itersPerImage, noiseStdev);
      const auto obj = CreateObjectiveFunction(trainingImagePairs);
      std::vector<double> parameters(ParameterCount);

      // default
      parameters[BandpassTypeParameter] = static_cast<int>(mBandpassType);
      parameters[BandpassLParameter] = mBandpassL;
      parameters[BandpassHParameter] = mBandpassH;
      parameters[InterpolationTypeParameter] = static_cast<int>(mInterpolationType);
      parameters[WindowTypeParameter] = static_cast<int>(mWindowType);
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
  void PlotUpsampleCoefficientAccuracyDependence(const std::string& trainingImagesDirectory, float maxShift, float noiseStdev, int itersPerImage, int iters) const
  {
    const auto trainingImages = LoadImages(trainingImagesDirectory);
    const auto trainingImagePairs = CreateImagePairs(trainingImages, maxShift, itersPerImage, noiseStdev);
    const auto obj = CreateObjectiveFunction(trainingImagePairs);

    std::vector<double> upsampleCoeff(iters);
    std::vector<double> accuracy(iters);
    const double xmin = 1;
    const double xmax = 35;
    std::atomic<int> progress = 0;

    for (int i = 0; i < iters; ++i)
    {
      LOG_INFO("Calculating upsample coeffecient accuracy dependence ({:.1f}%)", (float)progress / (iters - 1) * 100);
      std::vector<double> parameters(ParameterCount);

      // default
      parameters[BandpassTypeParameter] = static_cast<int>(mBandpassType);
      parameters[BandpassLParameter] = mBandpassL;
      parameters[BandpassHParameter] = mBandpassH;
      parameters[InterpolationTypeParameter] = static_cast<int>(mInterpolationType);
      parameters[WindowTypeParameter] = static_cast<int>(mWindowType);
      parameters[UpsampleCoeffParameter] = mUpsampleCoeff;
      parameters[L1ratioParameter] = mL1ratio;

      // modified
      parameters[UpsampleCoeffParameter] = xmin + (double)i / (iters - 1) * (xmax - xmin);

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
  void PlotNoiseAccuracyDependence(const std::string& trainingImagesDirectory, float maxShift, float noiseStdev, int itersPerImage, int iters) const
  {
    if (noiseStdev <= 0.0f)
    {
      LOG_ERROR("Please set some non-zero positive max noise stdev");
      return;
    }

    const auto trainingImages = LoadImages(trainingImagesDirectory);
    std::vector<double> noiseStdevs(iters);
    std::vector<double> accuracy(iters);
    const double xmin = 0;
    const double xmax = noiseStdev;
    std::atomic<int> progress = 0;

#pragma omp parallel for
    for (int i = 0; i < iters; ++i)
    {
      LOG_INFO("Calculating noise stdev accuracy dependence ({:.1f}%)", (float)progress / (iters - 1) * 100);
      float noise = xmin + (double)i / (iters - 1) * (xmax - xmin);
      const auto trainingImagePairs = CreateImagePairs(trainingImages, maxShift, itersPerImage, noise);
      const auto obj = CreateObjectiveFunction(trainingImagePairs);
      std::vector<double> parameters(ParameterCount);

      // default
      parameters[BandpassTypeParameter] = static_cast<int>(mBandpassType);
      parameters[BandpassLParameter] = mBandpassL;
      parameters[BandpassHParameter] = mBandpassH;
      parameters[InterpolationTypeParameter] = static_cast<int>(mInterpolationType);
      parameters[WindowTypeParameter] = static_cast<int>(mWindowType);
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
  void PlotNoiseOptimalBPHDependence(const std::string& trainingImagesDirectory, float maxShift, float noiseStdev, int itersPerImage, int iters) const
  {
    if (noiseStdev <= 0.0f)
    {
      LOG_ERROR("Please set some non-zero positive max noise stdev");
      return;
    }

    std::vector<double> noiseStdevs(iters);
    std::vector<double> optimalBPHs(iters);
    const double xmin = 0;
    const double xmax = noiseStdev;
    int progress = 0;

    for (int i = 0; i < iters; ++i)
    {
      LOG_INFO("Calculating noise stdev optimal BPH dependence ({:.1f}%)", (float)progress / (iters - 1) * 100);
      float noise = xmin + (double)i / (iters - 1) * (xmax - xmin);

      IterativePhaseCorrelation ipc = *this; // copy this
      ipc.Optimize(trainingImagesDirectory, trainingImagesDirectory, 2.0f, noise, itersPerImage, 0.0f, ParameterCount * 2, true);

      noiseStdevs[i] = noise;
      optimalBPHs[i] = ipc.GetBandpassH();
      progress++;
    }

    Plot1D::Set("NoiseOptimalBPHDependence");
    Plot1D::SetXlabel("Noise stdev");
    Plot1D::SetYlabel("Optimal BPH");
    Plot1D::Plot("NoiseOptimalBPHDependence", noiseStdevs, optimalBPHs);
  }

private:
  int mRows = 0;
  int mCols = 0;
  double mBandpassL = 0;
  double mBandpassH = 1;
  int mL2size = 11;
  double mL1ratio = 0.35;
  double mL1ratioStep = 0.05;
  int mUpsampleCoeff = 51;
  int mMaxIterations = 20;
  BandpassType mBandpassType = BandpassType::Gaussian;
  InterpolationType mInterpolationType = InterpolationType::Linear;
  WindowType mWindowType = WindowType::Hann;
  std::string mDebugDirectory = "Debug";
  std::string mDebugName = "IPC";
  mutable AccuracyType mAccuracyType = AccuracyType::SubpixelIterative;
  cv::Mat mBandpass;
  cv::Mat mFrequencyBandpass;
  cv::Mat mWindow;

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
      if (mBandpassL <= 0 && mBandpassH < 1)
      {
        for (int r = 0; r < mRows; ++r)
          for (int c = 0; c < mCols; ++c)
            mBandpass.at<float>(r, c) = LowpassEquation(r, c);
      }
      else if (mBandpassL > 0 && mBandpassH >= 1)
      {
        for (int r = 0; r < mRows; ++r)
          for (int c = 0; c < mCols; ++c)
            mBandpass.at<float>(r, c) = HighpassEquation(r, c);
      }
      else if (mBandpassL > 0 && mBandpassH < 1)
      {
        for (int r = 0; r < mRows; ++r)
          for (int c = 0; c < mCols; ++c)
            mBandpass.at<float>(r, c) = BandpassGEquation(r, c);

        normalize(mBandpass, mBandpass, 0.0, 1.0, cv::NORM_MINMAX);
      }
      break;

    case BandpassType::Rectangular:
      if (mBandpassL < mBandpassH)
      {
        for (int r = 0; r < mRows; ++r)
          for (int c = 0; c < mCols; ++c)
            mBandpass.at<float>(r, c) = BandpassREquation(r, c);
      }
      break;
    }

    CalculateFrequencyBandpass();
  }
  float LowpassEquation(int row, int col) const
  {
    return exp(-1.0 / (2. * std::pow(mBandpassH, 2)) * (std::pow(col - mCols / 2, 2) / std::pow(mCols / 2, 2) + std::pow(row - mRows / 2, 2) / std::pow(mRows / 2, 2)));
  }
  float HighpassEquation(int row, int col) const
  {
    return 1.0 - exp(-1.0 / (2. * std::pow(mBandpassL, 2)) * (std::pow(col - mCols / 2, 2) / std::pow(mCols / 2, 2) + std::pow(row - mRows / 2, 2) / std::pow(mRows / 2, 2)));
  }
  float BandpassGEquation(int row, int col) const { return LowpassEquation(row, col) * HighpassEquation(row, col); }
  float BandpassREquation(int row, int col) const
  {
    double r = sqrt(0.5 * (std::pow(col - mCols / 2, 2) / std::pow(mCols / 2, 2) + std::pow(row - mRows / 2, 2) / std::pow(mRows / 2, 2)));
    return (mBandpassL <= r && r <= mBandpassH) ? 1 : 0;
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
  cv::Mat CalculateFourierTransform(cv::Mat&& image) const
  {
    if constexpr (CudaFFT)
      return Fourier::cufft(std::move(image), PackedFFT);
    return Fourier::fft(std::move(image), PackedFFT);
  }
  cv::Mat CalculateCrossPowerSpectrum(cv::Mat&& dft1, cv::Mat&& dft2) const
  {
    if constexpr (PackedFFT)
    {
      cv::Mat cps;
      mulSpectrums(dft2, dft1, cps, 0, true);
      for (int row = 0; row < cps.rows; ++row)
      {
        for (int col = 0; col < cps.cols; ++col)
        {
          // for CCS packed FFT the cps vector is real, not complex
          // calculating magnitude & dividing is non-trivial
          // see magSpectrum() & divSpectrums() in OpenCV phaseCorrelate()
        }
      }
      return cps;
    }

    for (int row = 0; row < dft1.rows; ++row)
    {
      auto dft1p = dft1.ptr<cv::Vec2f>(row);
      auto dft2p = dft2.ptr<cv::Vec2f>(row);
      for (int col = 0; col < dft1.cols; ++col)
      {
        const float re = dft1p[col][0] * dft2p[col][0] + dft1p[col][1] * dft2p[col][1];
        const float im = dft1p[col][0] * dft2p[col][1] - dft1p[col][1] * dft2p[col][0];
        const float mag = sqrt(re * re + im * im);

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
    if (mBandpassL <= 0 && mBandpassH >= 1)
      return;

    if constexpr (PackedFFT)
      multiply(crosspower, mFrequencyBandpass(cv::Rect(0, 0, crosspower.cols, crosspower.rows)), crosspower);
    else
      multiply(crosspower, mFrequencyBandpass, crosspower);
  }
  void CalculateFrequencyBandpass()
  {
    mFrequencyBandpass = Fourier::dupchansc(mBandpass);
    Fourier::ifftshift(mFrequencyBandpass);
  }
  cv::Mat CalculateL3(cv::Mat&& crosspower) const
  {
    cv::Mat L3;
    if constexpr (CudaFFT)
      L3 = Fourier::icufft(std::move(crosspower), PackedFFT);
    else
      L3 = Fourier::ifft(std::move(crosspower), PackedFFT);

    Fourier::fftshift(L3);
    return L3;
  }
  cv::Point2f GetPeak(const cv::Mat& mat) const
  {
    cv::Point2i peak;
    minMaxLoc(mat, nullptr, nullptr, nullptr, &peak);

    if (peak.x < 0 || peak.y < 0 || peak.x >= mat.cols || peak.y >= mat.rows)
      return cv::Point2f(0, 0);

    return peak;
  }
  cv::Point2f GetPeakSubpixel(const cv::Mat& mat) const
  {
    double M = 0;
    double My = 0;
    double Mx = 0;

    for (int r = 0; r < mat.rows; ++r)
    {
      auto matp = mat.ptr<float>(r);
      for (int c = 0; c < mat.cols; ++c)
      {
        M += matp[c];
        My += matp[c] * r;
        Mx += matp[c] * c;
      }
    }

    cv::Point2f result(Mx / M, My / M);

    if (result.x < 0 || result.y < 0 || result.x >= mat.cols || result.y >= mat.rows)
      return cv::Point2f(mat.cols / 2, mat.rows / 2);

    return result;
  }
  cv::Mat CalculateL2(const cv::Mat& L3, const cv::Point2f& L3peak, int L2size) const { return roicrop(L3, L3peak.x, L3peak.y, L2size, L2size); }
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
  int GetL1size(const cv::Mat& L2U, double L1ratio) const
  {
    int L1size = std::floor(L1ratio * L2U.cols);
    L1size = L1size % 2 ? L1size : L1size + 1;
    return L1size;
  }
  cv::Mat CalculateL1(const cv::Mat& L2U, const cv::Point2f& L2Upeak, int L1size) const { return kirklcrop(L2U, L2Upeak.x, L2Upeak.y, L1size); }
  bool IsOutOfBounds(const cv::Point2i& peak, const cv::Mat& mat, int size) const { return IsOutOfBounds(peak, mat, {size, size}); }
  bool IsOutOfBounds(const cv::Point2i& peak, const cv::Mat& mat, cv::Size size) const
  {
    return peak.x - size.width / 2 < 0 || peak.y - size.height / 2 < 0 || peak.x + size.width / 2 >= mat.cols || peak.y + size.height / 2 >= mat.rows;
  }
  bool AccuracyReached(const cv::Point2f& L1peak, const cv::Point2f& L1mid) const { return abs(L1peak.x - L1mid.x) < 0.5 && abs(L1peak.y - L1mid.y) < 0.5; }
  bool ReduceL2size(int& L2size) const
  {
    L2size -= 2;
    if constexpr (DebugMode)
      LOG_WARNING("L2 out of bounds - reducing L2size to {}", L2size);
    return L2size >= 3;
  }
  void ReduceL1ratio(double& L1ratio) const
  {
    L1ratio -= mL1ratioStep;
    if constexpr (DebugMode)
      LOG_WARNING("L1 did not converge - reducing L1ratio to {:.2f}", L1ratio);
  }
  static cv::Mat ColorComposition(const cv::Mat& img1, const cv::Mat& img2)
  {
    const cv::Vec3f img1clr = {1, 0.5, 0};
    const cv::Vec3f img2clr = {0, 0.5, 1};

    const float gamma1 = 1.0;
    const float gamma2 = 1.0;

    cv::Mat img1c = cv::Mat::zeros(img1.size(), CV_32FC3);
    cv::Mat img2c = cv::Mat::zeros(img2.size(), CV_32FC3);

    for (int row = 0; row < img1.rows; ++row)
    {
      auto img1p = img1.ptr<float>(row);
      auto img2p = img2.ptr<float>(row);
      auto img1cp = img1c.ptr<cv::Vec3f>(row);
      auto img2cp = img2c.ptr<cv::Vec3f>(row);

      for (int col = 0; col < img1.cols; ++col)
      {
        img1cp[col][0] = pow(img1clr[2] * img1p[col], 1. / gamma1);
        img1cp[col][1] = pow(img1clr[1] * img1p[col], 1. / gamma1);
        img1cp[col][2] = pow(img1clr[0] * img1p[col], 1. / gamma1);

        img2cp[col][0] = pow(img2clr[2] * img2p[col], 1. / gamma2);
        img2cp[col][1] = pow(img2clr[1] * img2p[col], 1. / gamma2);
        img2cp[col][2] = pow(img2clr[0] * img2p[col], 1. / gamma2);
      }
    }

    if (gamma1 != 1 || gamma2 != 1)
    {
      normalize(img1c, img1c, 0, 1, cv::NORM_MINMAX);
      normalize(img2c, img2c, 0, 1, cv::NORM_MINMAX);
    }

    return (img1c + img2c) / 2;
  }

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
      static constexpr float cropFocusRatio = 0.5;
      auto image = loadImage(path);
      image = roicropmid(image, cropFocusRatio * image.cols, cropFocusRatio * image.rows);
      images.push_back(image);
      if (!mute)
        LOG_DEBUG("Loaded image {}", path);
    }
    return images;
  }
  std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>> CreateImagePairs(const std::vector<cv::Mat>& images, double maxShift, int itersPerImage, double noiseStdev) const
  {
    for (const auto& image : images)
    {
      if (image.rows < mRows + maxShift || image.cols < mCols + maxShift)
        throw std::runtime_error(fmt::format("Could not optimize IPC parameters - input image is too small for specified IPC window size & max shift ratio ([{},{}] < [{},{}])", image.rows, image.cols,
            mRows + maxShift, mCols + maxShift));
    }

    std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>> imagePairs;
    imagePairs.reserve(images.size() * itersPerImage);

    for (const auto& image : images)
    {
      for (int i = 0; i < itersPerImage; ++i)
      {
        // random shift from a random point
        cv::Point2f shift(rand11() * maxShift, rand11() * maxShift);
        cv::Point2i point(clamp(rand01() * image.cols, mCols, image.cols - mCols), clamp(rand01() * image.rows, mRows, image.rows - mRows));
        cv::Mat T = (cv::Mat_<float>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
        cv::Mat imageShifted;
        warpAffine(image, imageShifted, T, image.size());
        cv::Mat image1 = roicrop(image, point.x, point.y, mCols, mRows);
        cv::Mat image2 = roicrop(imageShifted, point.x, point.y, mCols, mRows);

        ConvertToUnitFloat(image1);
        ConvertToUnitFloat(image2);

        AddNoise(image1, noiseStdev);
        AddNoise(image2, noiseStdev);

        imagePairs.push_back({image1, image2, shift});

        if constexpr (DebugMode)
        {
          cv::Mat hcct;
          hconcat(image1, image2, hcct);
          showimg(hcct, fmt::format("IPC optimization pair {}", i));
        }
      }
    }
    return imagePairs;
  }
  void AddNoise(cv::Mat& image, double noiseStdev) const
  {
    if (noiseStdev <= 0)
      return;

    cv::Mat noise = cv::Mat::zeros(image.rows, image.cols, CV_32F);
    randn(noise, 0, noiseStdev);
    image += noise;
  }
  const std::function<double(const std::vector<double>&)> CreateObjectiveFunction(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs) const
  {
    return [&](const std::vector<double>& params)
    {
      IterativePhaseCorrelation ipc(mRows, mCols);
      ipc.SetBandpassType(static_cast<BandpassType>((int)params[BandpassTypeParameter]));
      ipc.SetBandpassParameters(params[BandpassLParameter], params[BandpassHParameter]);
      ipc.SetInterpolationType(static_cast<InterpolationType>((int)params[InterpolationTypeParameter]));
      ipc.SetWindowType(static_cast<WindowType>((int)params[WindowTypeParameter]));
      ipc.SetUpsampleCoeff(params[UpsampleCoeffParameter]);
      ipc.SetL1ratio(params[L1ratioParameter]);

      if (std::floor(ipc.GetL2size() * ipc.GetUpsampleCoeff() * ipc.GetL1ratio()) < 3)
        return std::numeric_limits<double>::max();

      double avgerror = 0;
      for (const auto& [image1, image2, shift] : imagePairs)
      {
        const auto error = ipc.Calculate(image1, image2) - shift;
        avgerror += sqrt(error.x * error.x + error.y * error.y);
      }
      return imagePairs.size() > 0 ? avgerror / imagePairs.size() : 0.0f;
    };
  }
  std::vector<double> CalculateOptimalParameters(
      const std::function<double(const std::vector<double>&)>& obj, const std::function<double(const std::vector<double>&)>& valid, int populationSize, bool mute) const
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
  void ApplyOptimalParameters(const std::vector<double>& optimalParameters, bool mute)
  {
    if (optimalParameters.size() != ParameterCount)
      throw std::runtime_error("Cannot apply optimal parameters - wrong parameter count");

    SetBandpassType(static_cast<BandpassType>((int)optimalParameters[BandpassTypeParameter]));
    SetBandpassParameters(optimalParameters[BandpassLParameter], optimalParameters[BandpassHParameter]);
    SetInterpolationType(static_cast<InterpolationType>((int)optimalParameters[InterpolationTypeParameter]));
    SetWindowType(static_cast<WindowType>((int)optimalParameters[WindowTypeParameter]));
    SetUpsampleCoeff(optimalParameters[UpsampleCoeffParameter]);
    SetL1ratio(optimalParameters[L1ratioParameter]);

    if (!mute)
    {
      LOG_INFO("Final IPC BandpassType: {}",
          BandpassType2String(static_cast<BandpassType>((int)optimalParameters[BandpassTypeParameter]), optimalParameters[BandpassLParameter], optimalParameters[BandpassHParameter]));
      LOG_INFO("Final IPC BandpassL: {:.2f}", optimalParameters[BandpassLParameter]);
      LOG_INFO("Final IPC BandpassH: {:.2f}", optimalParameters[BandpassHParameter]);
      LOG_INFO("Final IPC InterpolationType: {}", InterpolationType2String(static_cast<InterpolationType>((int)optimalParameters[InterpolationTypeParameter])));
      LOG_INFO("Final IPC WindowType: {}", WindowType2String(static_cast<WindowType>((int)optimalParameters[WindowTypeParameter])));
      LOG_INFO("Final IPC UpsampleCoeff: {}", static_cast<int>(optimalParameters[UpsampleCoeffParameter]));
      LOG_INFO("Final IPC L1ratio: {:.2f}", optimalParameters[L1ratioParameter]);
    }
  }
  std::string BandpassType2String(BandpassType type, double bandpassL, double bandpassH) const
  {
    switch (type)
    {
    case BandpassType::Rectangular:
      if (mBandpassL <= 0 && mBandpassH < 1)
        return "Rectangular low pass";
      else if (mBandpassL > 0 && mBandpassH >= 1)
        return "Rectangular high pass";
      else if (mBandpassL > 0 && mBandpassH < 1)
        return "Rectangular band pass";
      else if (mBandpassL <= 0 && mBandpassH >= 1)
        return "Rectangular all pass";

    case BandpassType::Gaussian:
      if (mBandpassL <= 0 && mBandpassH < 1)
        return "Gaussian low pass";
      else if (mBandpassL > 0 && mBandpassH >= 1)
        return "Gaussian high pass";
      else if (mBandpassL > 0 && mBandpassH < 1)
        return "Gausian band pass";
      else if (mBandpassL <= 0 && mBandpassH >= 1)
        return "Gaussian all pass";
    }
    return "Unknown";
  }
  std::string WindowType2String(WindowType type) const
  {
    switch (type)
    {
    case WindowType::Rectangular:
      return "Rectangular";
    case WindowType::Hann:
      return "Hann";
    }
    return "Unknown";
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
    }
    return "Unknown";
  }
  void ShowOptimizationPlots(const std::vector<cv::Point2f>& shiftsReference, const std::vector<cv::Point2f>& shiftsPixel, const std::vector<cv::Point2f>& shiftsNonit, const std::vector<cv::Point2f>& shiftsBefore,
      const std::vector<cv::Point2f>& shiftsAfter) const
  {
    std::vector<double> shiftsXReference, shiftsXReferenceError;
    std::vector<double> shiftsXPixel, shiftsXPixelError;
    std::vector<double> shiftsXNonit, shiftsXNonitError;
    std::vector<double> shiftsXBefore, shiftsXBeforeError;
    std::vector<double> shiftsXAfter, shiftsXAfterError;

    for (int i = 0; i < shiftsReference.size(); ++i)
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
  double GetAverageAccuracy(const std::vector<cv::Point2f>& shiftsReference, const std::vector<cv::Point2f>& shifts) const
  {
    if (shiftsReference.size() != shifts.size())
      throw std::runtime_error("Reference shift vector has different size than calculated shift vector");

    double avgerror = 0;
    for (int i = 0; i < shifts.size(); ++i)
    {
      const auto error = shifts[i] - shiftsReference[i];
      avgerror += sqrt(error.x * error.x + error.y * error.y);
    }
    return avgerror / shifts.size();
  }
  void ShowRandomImagePair(const std::vector<std::tuple<cv::Mat, cv::Mat, cv::Point2f>>& imagePairs)
  {
    const auto& [img1, img2, shift] = imagePairs[static_cast<size_t>(rand01() * imagePairs.size())];
    cv::Mat concat;
    hconcat(img1, img2, concat);
    Plot2D::Set("Random image pair");
    Plot2D::SetColorMapType(QCPColorGradient::gpGrayscale);
    Plot2D::Plot("Random image pair", concat);
  }
  static double GetFractionalPart(double x) { return abs(x - std::floor(x)); }
};


