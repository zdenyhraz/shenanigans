#include "stdafx.h"
#include "IterativePhaseCorrelation.h"

IterativePhaseCorrelation::IterativePhaseCorrelation(int rows, int cols, double bandpassL, double bandpassH) : mRows(rows), mCols(cols), mBandpassL(bandpassL), mBandpassH(bandpassH)
{
  mWindow = edgemask(mRows, mCols);
  mBandpass = bandpassian(mRows, mCols, mBandpassL, mBandpassH);
  CalculateFrequencyBandpass();
}

void IterativePhaseCorrelation::SetBandpassParameters(double bandpassL, double bandpassH)
{
  mBandpassL = bandpassL;
  mBandpassH = bandpassH;
  mBandpass = bandpassian(mRows, mCols, mBandpassL, mBandpassH);
  CalculateFrequencyBandpass();
}

void IterativePhaseCorrelation::SetSize(int rows, int cols)
{
  mRows = rows;
  mCols = cols > 0 ? cols : rows;
  mWindow = edgemask(mRows, mCols);
  mBandpass = bandpassian(mRows, mCols, mBandpassL, mBandpassH);
  CalculateFrequencyBandpass();
}

cv::Point2f IterativePhaseCorrelation::Calculate(const Mat &image1, const Mat &image2) const
{
  Mat img1 = image1.clone();
  Mat img2 = image2.clone();
  return Calculate(std::move(img1), std::move(img2));
}

inline cv::Point2f IterativePhaseCorrelation::Calculate(Mat &&img1, Mat &&img2) const
{
  ConvertToUnitFloat(img1, img2);
  ApplyWindow(img1, img2);
  auto [dft1, dft2] = CalculateFourierTransforms(img1, img2);
  auto crosspower = CalculateCrossPowerSpectrum(dft1, dft2);
  ApplyBandpass(crosspower);
  Mat L3 = CalculateL3(crosspower);
  auto [L3peak, L3max] = GetPeak(L3);
  Point2f L3mid(L3.cols / 2, L3.rows / 2);
  Point2f result = L3peak - L3mid;

  if (!mSubpixelEstimation)
    return result;

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

inline void IterativePhaseCorrelation::ConvertToUnitFloat(Mat &img1, Mat &img2) const
{
  double max1, max2;
  minMaxLoc(img1, nullptr, &max1);
  minMaxLoc(img2, nullptr, &max2);
  img1.convertTo(img1, CV_32F, 1. / max1);
  img2.convertTo(img2, CV_32F, 1. / max2);
}

inline void IterativePhaseCorrelation::ApplyWindow(Mat &img1, Mat &img2) const
{
  if (!mApplyWindow)
    return;

  multiply(img1, mWindow, img1);
  multiply(img2, mWindow, img2);
}

inline std::pair<Mat, Mat> IterativePhaseCorrelation::CalculateFourierTransforms(Mat &img1, Mat &img2) const { return std::make_pair(fourier(std::move(img1)), fourier(std::move(img2))); }

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

  if (!mCrossCorrelate)
  {
    Mat magnre, magnim;
    pow(CrossPowerPlanes[0], 2, magnre);
    pow(CrossPowerPlanes[1], 2, magnim);
    Mat normalizationdenominator = magnre + magnim;
    sqrt(normalizationdenominator, normalizationdenominator);
    CrossPowerPlanes[0] /= (normalizationdenominator + mDivisionEpsilon);
    CrossPowerPlanes[1] /= (normalizationdenominator + mDivisionEpsilon);
  }

  Mat CrossPower;
  merge(CrossPowerPlanes, 2, CrossPower);
  return CrossPower;
}

inline void IterativePhaseCorrelation::ApplyBandpass(Mat &crosspower) const
{
  if (!mApplyBandpass)
    return;

  multiply(crosspower, mFrequencyBandpass, crosspower);
}

void IterativePhaseCorrelation::CalculateFrequencyBandpass()
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

inline std::pair<Point2f, double> IterativePhaseCorrelation::GetPeak(const Mat &mat) const
{
  Point2i matpeak;
  double matmax;
  minMaxLoc(mat, nullptr, &matmax, nullptr, &matpeak);
  return std::make_pair(matpeak, matmax);
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

inline Mat IterativePhaseCorrelation::CalculateL2(const Mat &L3, const Point2f &L3peak, int L2size) const { return roicrop(L3, L3peak.x, L3peak.y, L2size, L2size); }

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

inline Mat IterativePhaseCorrelation::CalculateL1(const Mat &L2U, const Point2f &L2Upeak, int L1size) const { return kirklcrop(L2U, L2Upeak.x, L2Upeak.y, L1size); }

inline bool IterativePhaseCorrelation::IsOutOfBounds(const Point2f &peak, const Mat &mat, int size) const { return ((peak.x - size / 2) < 0) || ((peak.y - size / 2) < 0) || ((peak.x + size / 2) >= mat.cols) || ((peak.y + size / 2) >= mat.rows); }

inline bool IterativePhaseCorrelation::AccuracyReached(const Point2f &L1peak, const Point2f &L1mid) const { return (abs(L1peak.x - L1mid.x) < 0.5) && (abs(L1peak.y - L1mid.y) < 0.5); }

inline bool IterativePhaseCorrelation::ReduceL2size(int &L2size) const
{
  L2size -= 2;
  return L2size >= 3;
}
