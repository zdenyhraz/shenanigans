#include "stdafx.h"
#include "IterativePhaseCorrelation.h"
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Fourier/fourier.h"
#include "Astrophysics/FITS.h"
#include "Filtering/filtering.h"
#include "Log/logger.h"
#include "Optimization/Evolution.h"

IterativePhaseCorrelation::IterativePhaseCorrelation(int rows, int cols, double bandpassL, double bandpassH)
: mRows(rows), mCols(cols), mBandpassL(bandpassL), mBandpassH(bandpassH)
{
  UpdateWindow();
  UpdateBandpass();
}

void IterativePhaseCorrelation::SetBandpassParameters(double bandpassL, double bandpassH)
{
  mBandpassL = bandpassL;
  mBandpassH = bandpassH;
  UpdateBandpass();
}

void IterativePhaseCorrelation::SetSize(int rows, int cols)
{
  mRows = rows;
  mCols = cols > 0 ? cols : rows;
  UpdateWindow();
  UpdateBandpass();
}

Point2f IterativePhaseCorrelation::Calculate(const Mat &image1, const Mat &image2) const
{
  Mat img1 = image1.clone();
  Mat img2 = image2.clone();
  return Calculate(std::move(img1), std::move(img2));
}

inline Point2f IterativePhaseCorrelation::Calculate(Mat &&img1, Mat &&img2) const
{
  if (!IsValid(img1, img2))
    return {0, 0};

  ConvertToUnitFloat(img1, img2);
  ApplyWindow(img1, img2);
  auto [dft1, dft2] = CalculateFourierTransforms(img1, img2);
  auto crosspower = CalculateCrossPowerSpectrum(dft1, dft2);
  ApplyBandpass(crosspower);

  Mat L3 = CalculateL3(crosspower);
  Point2f L3peak = GetPeak(L3);
  Point2f L3mid(L3.cols / 2, L3.rows / 2);
  Point2f result = L3peak - L3mid;

  int L2size = mL2size;
  bool converged = false;
  while (!converged)
  {
    if (IsOutOfBounds(L3peak, L3, L2size))
      if (!ReduceL2size(L2size))
        break;

    // L2
    Mat L2 = CalculateL2(L3, L3peak, L2size);
    Point2f L2mid(L2.cols / 2, L2.rows / 2);

    // L2U
    Mat L2U = CalculateL2U(L2);
    Point2f L2Umid(L2U.cols / 2, L2U.rows / 2);
    Point2f L2Upeak = L2Umid;

    // L1
    int L1size = GetL1size(L2U);
    Mat L1;
    Point2f L1mid(L1size / 2, L1size / 2);
    Point2f L1peak;

    for (int iter = 0; iter < mMaxIterations; ++iter)
    {
      L1 = CalculateL1(L2U, L2Upeak, L1size);
      L1peak = GetPeakSubpixel(L1);
      L2Upeak += Point2f(round(L1peak.x - L1mid.x), round(L1peak.y - L1mid.y));

      if (IsOutOfBounds(L2Upeak, L2U, L1size))
        break;

      if (AccuracyReached(L1peak, L1mid))
      {
        L1 = CalculateL1(L2U, L2Upeak, L1size);
        converged = true;
        break;
      }
    }

    if (converged)
    {
      result = L3peak - L3mid + (L2Upeak - L2Umid + GetPeakSubpixel(L1) - L1mid) / mUpsampleCoeff;
      break;
    }
    else if (!ReduceL2size(L2size))
      break;
  }
  return result;
}

void IterativePhaseCorrelation::Optimize(const std::string &trainingImagesDirectory, const std::string &validationImagesDirectory,
    float maxShiftRatio, float noiseStdev, int itersPerImage)
{
  // arguments sanity checks
  if (itersPerImage < 1)
  {
    LOG_ERROR("Could not optimize IPC parameters - invalid iters per image ({})", itersPerImage);
    return;
  }

  if (maxShiftRatio >= 1)
  {
    LOG_ERROR("Could not optimize IPC parameters - invalid max shift ratio ({})", maxShiftRatio);
    return;
  }

  if (noiseStdev < 0)
  {
    LOG_ERROR("Could not optimize IPC parameters - noise stdev cannot be negative ({})", noiseStdev);
    return;
  }

  // load training images
  LOG_INFO("Loading training images from '{}'...", trainingImagesDirectory);
  std::vector<Mat> trainingImages;
  if (std::filesystem::is_directory(trainingImagesDirectory))
  {
    for (const auto &entry : std::filesystem::directory_iterator(trainingImagesDirectory))
    {
      std::string path = entry.path().string();

      if (!IsImage(path))
        continue;

      trainingImages.push_back(loadImage(path));
      LOG_DEBUG("Loaded training image {}", path);
    }
  }
  else
  {
    LOG_ERROR("Could not optimize IPC parameters - trainign directory '{}' is not a valid directory", trainingImagesDirectory);
    return;
  }

  // load validation images
  LOG_INFO("Loading validation images from '{}'...", validationImagesDirectory);
  std::vector<Mat> validationImages;
  if (std::filesystem::is_directory(validationImagesDirectory))
  {
    for (const auto &entry : std::filesystem::directory_iterator(validationImagesDirectory))
    {
      std::string path = entry.path().string();

      if (!IsImage(path))
        continue;

      validationImages.push_back(loadImage(path));
      LOG_DEBUG("Loaded validation image '{}'", path);
    }
  }
  else
  {
    LOG_DEBUG("Optimizing IPC parameters without validation set - '{}' is not a valid directory", validationImagesDirectory);
  }

  LOG_INFO("Running Iterative Phase Correlation parameter optimization on a set of {} images...", trainingImages.size());

  if (trainingImages.empty())
  {
    LOG_ERROR("Could not optimize IPC parameters - empty input image vector");
    return;
  }

  for (const auto &image : trainingImages)
  {
    const Point2f maxShift{maxShiftRatio * mCols, maxShiftRatio * mRows};
    if (image.rows < mRows + maxShift.y || image.cols < mCols + maxShift.x)
    {
      LOG_ERROR("Could not optimize IPC parameters - input image is too small for specified IPC window size & max shift ratio ([{},{}] < [{},{}])",
          image.rows, image.cols, mRows + maxShift.y, mCols + maxShift.x);
      return;
    }
  }

  // prepare the shifted image pairs
  // shift each image n times up to max shift ratio
  // both images are roicropped in the middle but 2nd image is shifted beforehands
  std::vector<std::tuple<Mat, Mat, Point2f>> imagePairs;
  imagePairs.reserve(trainingImages.size() * itersPerImage);
  for (const auto &image : trainingImages)
  {
    const Point2f maxShift{maxShiftRatio * mCols, maxShiftRatio * mRows};
    for (int i = 0; i < itersPerImage; ++i)
    {
      // create the shifted image pair
      Mat image1 = roicrop(image, image.cols / 2, image.rows / 2, mCols, mRows);
      Mat image2;
      float r = (float)i / (itersPerImage - 1);
      Point2f shift = r * maxShift;
      Mat T = (Mat_<float>(2, 3) << 1., 0., shift.x, 0., 1., shift.y);
      warpAffine(image, image2, T, image.size());
      image2 = roicrop(image2, image2.cols / 2, image2.rows / 2, mCols, mRows);

      // convert both pictures to unit float
      image1.convertTo(image1, CV_32F);
      image2.convertTo(image2, CV_32F);
      normalize(image1, image1, 0, 1, CV_MINMAX);
      normalize(image2, image2, 0, 1, CV_MINMAX);

      // add noise
      if (noiseStdev > 0)
      {
        Mat noise1 = Mat::zeros(image1.rows, image1.cols, CV_32F);
        Mat noise2 = Mat::zeros(image2.rows, image2.cols, CV_32F);
        randn(noise1, 0, noiseStdev);
        randn(noise2, 0, noiseStdev);
        image1 += noise1;
        image2 += noise2;
      }

      imagePairs.push_back({image1, image2, shift});
    }
  }

  // the average registration error objective function
  const auto f = [&](const std::vector<double> &params) {
    // create ipc object with speficied parameters
    IterativePhaseCorrelation ipc(mRows, mCols);
    ipc.SetBandpassParameters(params[0], params[1]);
    ipc.SetL2size(params[2]);
    ipc.SetUpsampleCoeff(params[3]);
    ipc.SetInterpolationType(params[4] > 0 ? INTER_CUBIC : INTER_LINEAR);
    // void SetBandpassType(BandpassType type) { mBandpassType = type; }
    // void SetWindowType(WindowType type) { mWindowType = type; }
    ipc.SetL1ratio(params[7]);

    // calculate average registration error
    double avgerror = 0;
    for (const auto &[image1, image2, shift] : imagePairs)
    {
      const auto error = ipc.Calculate(image1, image2) - shift;
      avgerror += sqrt(error.x * error.x + error.y * error.y);
    }
    return avgerror / imagePairs.size();
  };

  // get the best parameters via differential evolution
  int N = 8;
  Evolution evo(N);
  evo.mNP = 50;
  evo.mMutStrat = Evolution::RAND1;
  evo.SetParameterNames({"BPL", "BPH", "L2", "UC", "INTERP", "WINDOW", "BANDPASS", "L1RATIO"});
  evo.mLB = {0.0001, 0.0001, 3, 5, -1, -1, -1, 0.1};
  evo.mUB = {5.0, 1.0, 21, 51, +1, +1, +1, 0.9};
  const auto bestParams = evo.Optimize(f);

  // set the currently used parameters to the best parameters and show the resulting bandpass, window, etc.
  if (bestParams.size() == N)
  {
    SetBandpassParameters(bestParams[0], bestParams[1]);
    SetL2size(bestParams[2]);
    SetUpsampleCoeff(bestParams[3]);
    SetInterpolationType(bestParams[4] > 0 ? INTER_CUBIC : INTER_LINEAR);
    // void SetBandpassType(BandpassType type) { mBandpassType = type; }
    // void SetWindowType(WindowType type) { mWindowType = type; }
    ShowDebugStuff();
    LOG_SUCC("Iterative Phase Correlation parameter optimization successful");
  }
  else
  {
    LOG_ERROR("Iterative Phase Correlation parameter optimization failed");
  }
}

void IterativePhaseCorrelation::ShowDebugStuff() const
{
  std::vector<double> x(mCols);
  std::vector<double> bandpass1D(mCols);
  std::vector<double> window1D(mCols);

  for (int c = 0; c < mCols; ++c)
  {
    x[c] = c + 1;
    bandpass1D[c] = mBandpass.at<float>(mRows / 2, c);
    window1D[c] = mWindow.at<float>(mRows / 2, c);
  }

  Plot1D::plot(x, bandpass1D, "IPC bandpass 1D", "x", "IPC bandpass");
  Plot1D::plot(x, window1D, "IPC window 1D", "x", "IPC window");

  Plot2D::plot(mBandpass, "IPC bandpass", "x", "y", "IPC bandpass", 1, mCols, 1, mRows);
  Plot2D::plot(mWindow, "IPC window", "x", "y", "IPC window", 1, mCols, 1, mRows);
}

inline void IterativePhaseCorrelation::UpdateWindow()
{
  switch (mWindowType)
  {
  case WindowType::Rectangular:
    mWindow = Mat::ones(mRows, mCols, CV_32F);
    break;

  case WindowType::Hann:
    createHanningWindow(mWindow, cv::Size(mCols, mRows), CV_32F);
    break;
  }
}

inline void IterativePhaseCorrelation::UpdateBandpass()
{
  mBandpass = Mat::ones(mRows, mCols, CV_32F);

  switch (mBandpassType)
  {
  case BandpassType::Gaussian:
    if (mBandpassL > 0 && mBandpassH <= 0)
    {
      for (int r = 0; r < mRows; ++r)
        for (int c = 0; c < mCols; ++c)
          mBandpass.at<float>(r, c) = BandpassLEquation(r, c);
    }
    else if (mBandpassL <= 0 && mBandpassH > 0)
    {
      for (int r = 0; r < mRows; ++r)
        for (int c = 0; c < mCols; ++c)
          mBandpass.at<float>(r, c) = BandpassHEquation(r, c);
    }
    else if (mBandpassL > 0 && mBandpassH > 0)
    {
      for (int r = 0; r < mRows; ++r)
        for (int c = 0; c < mCols; ++c)
          mBandpass.at<float>(r, c) = BandpassGEquation(r, c);
    }

    if (mBandpassL > 0 || mBandpassH > 0)
      normalize(mBandpass, mBandpass, 0.0, 1.0, NORM_MINMAX);
    break;

  case BandpassType::Rectangular:
    for (int r = 0; r < mRows; ++r)
      for (int c = 0; c < mCols; ++c)
        mBandpass.at<float>(r, c) = BandpassREquation(r, c);
    break;
  }

  CalculateFrequencyBandpass();
}

inline float IterativePhaseCorrelation::BandpassLEquation(int row, int col) const
{
  return exp(-std::pow(mBandpassL, 2) * (std::pow(((float)col - mCols / 2) / (mCols / 2), 2) + std::pow(((float)row - mRows / 2) / (mRows / 2), 2)));
}

inline float IterativePhaseCorrelation::BandpassHEquation(int row, int col) const
{
  return 1.0 - exp(-1.0 / std::pow(mBandpassH, 2) *
                   (std::pow(((float)col - mCols / 2) / (mCols / 2), 2) + std::pow(((float)row - mRows / 2) / (mRows / 2), 2)));
}

inline float IterativePhaseCorrelation::BandpassGEquation(int row, int col) const
{
  return BandpassLEquation(row, col) * BandpassHEquation(row, col);
}

float IterativePhaseCorrelation::BandpassREquation(int row, int col) const
{
  if (mBandpassL >= mBandpassH)
    return 1;

  float R = sqrt(std::pow(((float)col - mCols / 2) / (mCols / 2), 2) + std::pow(((float)row - mRows / 2) / (mRows / 2), 2));
  return (mBandpassL <= R && R <= mBandpassH) ? 1 : 0;
}

inline bool IterativePhaseCorrelation::IsValid(const Mat &img1, const Mat &img2) const
{
  // size must be equal for matching bandpass / window
  if (!CheckSize(img1, img2))
    return false;

  // only grayscale images are supported
  if (!CheckChannels(img1, img2))
    return false;

  return true;
}

inline bool IterativePhaseCorrelation::CheckSize(const Mat &img1, const Mat &img2) const
{
  return img1.rows == mRows && img1.cols == mCols && img2.rows == mRows && img2.cols == mCols;
}

inline bool IterativePhaseCorrelation::CheckChannels(const Mat &img1, const Mat &img2) const { return img1.channels() == 1 && img2.channels() == 1; }

inline void IterativePhaseCorrelation::ConvertToUnitFloat(Mat &img1, Mat &img2) const
{
  img1.convertTo(img1, CV_32F);
  img2.convertTo(img2, CV_32F);
  normalize(img1, img1, 0, 1, CV_MINMAX);
  normalize(img2, img2, 0, 1, CV_MINMAX);
}

inline void IterativePhaseCorrelation::ApplyWindow(Mat &img1, Mat &img2) const
{
  multiply(img1, mWindow, img1);
  multiply(img2, mWindow, img2);
}

inline std::pair<Mat, Mat> IterativePhaseCorrelation::CalculateFourierTransforms(Mat &img1, Mat &img2) const
{
  return std::make_pair(fourier(std::move(img1)), fourier(std::move(img2)));
}

inline Mat IterativePhaseCorrelation::CalculateCrossPowerSpectrum(const Mat &dft1, const Mat &dft2) const
{
  Mat planes1[2];
  Mat planes2[2];
  Mat CrossPowerPlanes[2];

  split(dft1, planes1);
  split(dft2, planes2);

  planes1[1] *= -1;                                                              // complex conjugate of second pic
  CrossPowerPlanes[0] = planes1[0].mul(planes2[0]) - planes1[1].mul(planes2[1]); // pointwise multiplications real
  CrossPowerPlanes[1] = planes1[0].mul(planes2[1]) + planes1[1].mul(planes2[0]); // imag

  Mat magnre, magnim;
  pow(CrossPowerPlanes[0], 2, magnre);
  pow(CrossPowerPlanes[1], 2, magnim);
  Mat normalizationdenominator = magnre + magnim;
  sqrt(normalizationdenominator, normalizationdenominator);
  CrossPowerPlanes[0] /= normalizationdenominator;
  CrossPowerPlanes[1] /= normalizationdenominator;

  Mat CrossPower;
  merge(CrossPowerPlanes, 2, CrossPower);
  return CrossPower;
}

inline void IterativePhaseCorrelation::ApplyBandpass(Mat &crosspower) const { multiply(crosspower, mFrequencyBandpass, crosspower); }

inline void IterativePhaseCorrelation::CalculateFrequencyBandpass()
{
  Mat bandpassF = quadrantswap(mBandpass);
  Mat bandpassFplanes[2] = {bandpassF, bandpassF};
  merge(bandpassFplanes, 2, mFrequencyBandpass);
}

inline Mat IterativePhaseCorrelation::CalculateL3(const Mat &crosspower) const
{
  Mat L3;
  dft(crosspower, L3, DFT_INVERSE + DFT_SCALE + DFT_REAL_OUTPUT); // real only (assume pure real input)
  SwapQuadrants(L3);
  return L3;
}

inline void IterativePhaseCorrelation::SwapQuadrants(Mat &mat) const
{
  int centerX = mat.cols / 2;
  int centerY = mat.rows / 2;
  Mat q1(mat, Rect(0, 0, centerX, centerY));
  Mat q2(mat, Rect(centerX, 0, centerX, centerY));
  Mat q3(mat, Rect(0, centerY, centerX, centerY));
  Mat q4(mat, Rect(centerX, centerY, centerX, centerY));
  Mat temp;

  q1.copyTo(temp);
  q4.copyTo(q1);
  temp.copyTo(q4);

  q2.copyTo(temp);
  q3.copyTo(q2);
  temp.copyTo(q3);
}

inline Point2f IterativePhaseCorrelation::GetPeak(const Mat &mat) const
{
  Point2i peak;
  minMaxLoc(mat, nullptr, nullptr, nullptr, &peak);
  return peak;
}

inline Point2f IterativePhaseCorrelation::GetPeakSubpixel(const Mat &mat) const
{
  double M = 0;
  double My = 0;
  double Mx = 0;
  int r, c;

  for (r = 0; r < mat.rows; ++r)
  {
    for (c = 0; c < mat.cols; ++c)
    {
      M += mat.at<float>(r, c);
      My += mat.at<float>(r, c) * r;
      Mx += mat.at<float>(r, c) * c;
    }
  }

  Point2f result(Mx / M, My / M);

  if (result.x < 0 || result.y < 0 || result.x > mat.cols - 1 || result.y > mat.rows - 1)
    return Point2f(mat.cols / 2, mat.rows / 2);
  else
    return result;
}

inline Mat IterativePhaseCorrelation::CalculateL2(const Mat &L3, const Point2f &L3peak, int L2size) const
{
  return roicrop(L3, L3peak.x, L3peak.y, L2size, L2size);
}

inline Mat IterativePhaseCorrelation::CalculateL2U(const Mat &L2) const
{
  Mat L2U;
  resize(L2, L2U, L2.size() * mUpsampleCoeff, 0, 0, mInterpolationType);
  return L2U;
}

inline int IterativePhaseCorrelation::GetL1size(const Mat &L2U) const
{
  int L1size = std::floor(mL1ratio * L2U.cols);
  L1size = L1size % 2 ? L1size : L1size + 1;
  return L1size;
}

inline Mat IterativePhaseCorrelation::CalculateL1(const Mat &L2U, const Point2f &L2Upeak, int L1size) const
{
  return kirklcrop(L2U, L2Upeak.x, L2Upeak.y, L1size);
}

inline bool IterativePhaseCorrelation::IsOutOfBounds(const Point2f &peak, const Mat &mat, int size) const
{
  return ((peak.x - size / 2) < 0) || ((peak.y - size / 2) < 0) || ((peak.x + size / 2) >= mat.cols) || ((peak.y + size / 2) >= mat.rows);
}

inline bool IterativePhaseCorrelation::AccuracyReached(const Point2f &L1peak, const Point2f &L1mid) const
{
  return (abs(L1peak.x - L1mid.x) < 0.5) && (abs(L1peak.y - L1mid.y) < 0.5);
}

inline bool IterativePhaseCorrelation::ReduceL2size(int &L2size) const
{
  L2size -= 2;
  return L2size >= 3;
}
