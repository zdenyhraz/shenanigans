#include "stdafx.h"
#include "IterativePhaseCorrelation.h"

inline cv::Point2f IterativePhaseCorrelation::Calculate(const Mat &image1, const Mat &image2)
{
  Mat img1 = image1.clone();
  Mat img2 = image2.clone();
  return Calculate(std::move(img1), std::move(img2));
}

inline cv::Point2f IterativePhaseCorrelation::Calculate(Mat &&img1, Mat &&img2)
{
  ConvertToUnitFloat(img1, img2);
  ApplyWindow(img1, img2);
  auto [dft1, dft2] = CalculateFourierTransforms(std::move(img1), std::move(img2));
  auto crosspower = CalculateCrossPowerSpectrum(dft1, dft2);
  ApplyBandpass(crosspower);
  Mat L3 = CalculateL3(crosspower);
  auto [L3peak, L3max] = GetPeak(L3);
  Point2f L3mid(L3.cols / 2, L3.rows / 2);
  Point2f result(L3peak.x - L3mid.x, L3peak.y - L3mid.y);

  if (mSubpixelEstimation)
  {
    bool converged = false;

    while (!converged)
    {
      if (((L3peak.x - mL2size / 2) < 0) || ((L3peak.y - mL2size / 2) < 0) || ((L3peak.x + mL2size / 2) >= L3.cols) || ((L3peak.y + mL2size / 2) >= L3.rows))
      {
        LOG_ERROR("Degenerate peak (Imgsize=[{},{}],L3peak=[{},{}],mL2size=[{},{}]) - results might be inaccurate, reducing mL2size from {} to {} ", L3.cols, L3.rows, L3peak.x, L3peak.y, mL2size, mL2size, mL2size, mL2size - 2);
        mL2size -= 2;
        if (mL2size < 3)
        {
          LOG_ERROR("Completely degenerate peak, returning just with pixel accuracy");
          break;
        }
      }
      else
      {
        Mat L2 = roicrop(L3, L3peak.x, L3peak.y, mL2size, mL2size);
        Mat L2U;

        if (mInterpolate)
          resize(L2, L2U, L2.size() * mUpsampleCoeff, 0, 0, INTER_LINEAR);
        else
          resize(L2, L2U, L2.size() * mUpsampleCoeff, 0, 0, INTER_NEAREST);

        Point2f L2mid(L2.cols / 2, L2.rows / 2);
        Point2f L2Umid(L2U.cols / 2, L2U.rows / 2);
        Point2f L2Upeak = L2Umid;
        LOG_DEBUG("L2Upeak location before iterations: {}", to_string(L2Upeak));
        LOG_DEBUG("L2Upeak location before iterations findCentroid double: {}", to_string(findCentroid(L2U)));
        int L1size = std::round((float)L2U.cols * mL1ratio);
        if (!(L1size % 2))
          L1size++; // odd!+
        Mat L1 = kirklcrop(L2U, L2Upeak.x, L2Upeak.y, L1size);
        Point2f L1mid(L1.cols / 2, L1.rows / 2);
        result = (Point2f)L3peak - L3mid + findCentroid(L1) - L1mid;

        for (int i = 0; i < mMaxIterations; i++)
        {
          LOG_DEBUG("======= iteration {} =======", i + 1);
          L1 = kirklcrop(L2U, L2Upeak.x, L2Upeak.y, L1size);
          Point2f L1peak = findCentroid(L1);
          LOG_DEBUG("L1peak: {}", to_string(L1peak));
          L2Upeak.x += round(L1peak.x - L1mid.x);
          L2Upeak.y += round(L1peak.y - L1mid.y);
          if ((L2Upeak.x > (L2U.cols - L1mid.x - 1)) || (L2Upeak.y > (L2U.rows - L1mid.y - 1)) || (L2Upeak.x < (L1mid.x + 1)) || (L2Upeak.y < (L1mid.y + 1)))
          {
            LOG_ERROR("IPC out of bounds - centroid diverged, reducing mL2size from {} to {} ", mL2size, mL2size - 2);
            mL2size += -2;
            break;
          }
          LOG_DEBUG("L1peak findCentroid double delta: {}/{}", to_string(findCentroid(L1).x - L1mid.x), to_string(findCentroid(L1).y - L1mid.y));
          LOG_DEBUG("Resulting L2Upeak in this iteration: {}", to_string(L2Upeak));
          if ((abs(L1peak.x - L1mid.x) < 0.5) && (abs(L1peak.y - L1mid.y) < 0.5))
          {
            L1 = kirklcrop(L2U, L2Upeak.x, L2Upeak.y, L1size);
            LOG_DEBUG("Iterative phase correlation accuracy reached, mL2size: {}, L2Upeak: " + to_string(L2Upeak), mL2size);
            converged = true;
            break;
          }
          else if (i == mMaxIterations - 1)
          {
            LOG_DEBUG("IPC centroid oscilated, reducing mL2size from {} to {} ", mL2size, mL2size - 2);
            mL2size += -2;
          }
        }

        if (converged)
        {
          result.x = (float)L3peak.x - (float)L3mid.x + 1.0 / (float)mUpsampleCoeff * ((float)L2Upeak.x - (float)L2Umid.x + findCentroid(L1).x - (float)L1mid.x); // image shift in L3 - final
          result.y = (float)L3peak.y - (float)L3mid.y + 1.0 / (float)mUpsampleCoeff * ((float)L2Upeak.y - (float)L2Umid.y + findCentroid(L1).y - (float)L1mid.y); // image shift in L3 - final
        }
        else if (mL2size < 3)
        {
          LOG_ERROR("IPC centroid did not converge with any mL2size");
          break;
        }
      }
    }
  }
  return result;
}

void IterativePhaseCorrelation::ConvertToUnitFloat(Mat &img1, Mat &img2)
{
  double max1, max2;
  minMaxLoc(img1, nullptr, &max1);
  minMaxLoc(img2, nullptr, &max2);
  img1.convertTo(img1, CV_32F, 1. / max1);
  img2.convertTo(img2, CV_32F, 1. / max2);
}

void IterativePhaseCorrelation::ApplyWindow(Mat &img1, Mat &img2)
{
  if (!mApplyWindow)
    return;

  multiply(img1, mWindow, img1);
  multiply(img2, mWindow, img2);
}

std::pair<Mat, Mat> IterativePhaseCorrelation::CalculateFourierTransforms(Mat &&img1, Mat &&img2) { return std::make_pair(fourier(std::move(img1)), fourier(std::move(img2))); }

Mat IterativePhaseCorrelation::CalculateCrossPowerSpectrum(const Mat &dft1, const Mat &dft2)
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

void IterativePhaseCorrelation::ApplyBandpass(Mat &crosspower)
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

Mat IterativePhaseCorrelation::CalculateL3(const Mat &crosspower)
{
  // real only (assume pure real input)
  Mat L3;
  dft(crosspower, L3, DFT_INVERSE + DFT_SCALE + DFT_REAL_OUTPUT);
  SwapQuadrants(L3);
  return L3;
}

void IterativePhaseCorrelation::SwapQuadrants(Mat &mat)
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

std::pair<Point2i, double> IterativePhaseCorrelation::GetPeak(const Mat &mat)
{
  Point2i matpeak;
  double matmax;
  minMaxLoc(mat, nullptr, &matmax, nullptr, &matpeak);
  return std::make_pair(matpeak, matmax);
}
