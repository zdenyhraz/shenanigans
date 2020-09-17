#pragma once
#include "stdafx.h"
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Fourier/fourier.h"
#include "Astrophysics/FITS.h"
#include "Filtering/filtering.h"
#include "Log/logger.h"

class IterativePhaseCorrelation
{
public:
  IterativePhaseCorrelation(int rows, int cols, double bandpassL, double bandpassH);
  void SetSize(int rows, int cols = -1);
  void SetBandpassParameters(double bandpassL, double bandpassH);
  void SetL2size(int L2size) { mL2size = L2size % 2 ? L2size : L2size + 1; }
  void SetL1ratio(double L1ratio) { mL1ratio = L1ratio; }
  void SetUpsampleCoeff(int UpsampleCoeff) { mUpsampleCoeff = UpsampleCoeff; }
  void SetDivisionEpsilon(double DivisionEpsilon) { mDivisionEpsilon = DivisionEpsilon; }
  void SetMaxIterations(int MaxIterations) { mMaxIterations = MaxIterations; }
  void SetInterpolationType(InterpolationFlags InterpolationType) { mInterpolationType = InterpolationType; }
  void SetInterpolationType() {}
  void SetApplyWindow(bool ApplyWindow) { mApplyWindow = ApplyWindow; }
  void SetApplyBandpass(bool ApplyBandpass) { mApplyBandpass = ApplyBandpass; }
  void SetSubpixelEstimation(bool SubpixelEstimation) { mSubpixelEstimation = SubpixelEstimation; }
  void SetCrossCorrelate(bool CrossCorrelate) { mCrossCorrelate = CrossCorrelate; }

  int GetRows() const { return mRows; }
  int GetCols() const { return mCols; }
  int GetSize() const { return mRows; }
  double GetBandpassL() const { return mBandpassL; }
  double GetBandpassH() const { return mBandpassH; }
  int GetL2size() const { return mL2size; }
  int GetL1ratio() const { return mL1ratio; }
  int GetUpsampleCoeff() const { return mUpsampleCoeff; }
  Mat GetWindow() const { return mWindow; }
  Mat GetBandpass() const { return mBandpass; }

  cv::Point2f Calculate(const Mat &image1, const Mat &image2) const;
  cv::Point2f Calculate(Mat &&image1, Mat &&image2) const;

private:
  int mRows = 0;
  int mCols = 0;
  double mBandpassL = 1;
  double mBandpassH = 200;
  double mL1ratio = 0.35;
  int mL2size = 15;
  int mUpsampleCoeff = 51;
  double mDivisionEpsilon = 0;
  int mMaxIterations = 20;
  InterpolationFlags mInterpolationType = INTER_LINEAR;
  bool mApplyWindow = true;
  bool mApplyBandpass = true;
  bool mSubpixelEstimation = true;
  bool mCrossCorrelate = false;
  Mat mBandpass;
  Mat mFrequencyBandpass;
  Mat mWindow;
  bool mSave = false;
  string mSavedir = "";
  cv::Size mSavesize = cv::Size(500, 500);
  int mSavecntr = 0;

  void ConvertToUnitFloat(Mat &img1, Mat &img2) const;
  void ApplyWindow(Mat &img1, Mat &img2) const;
  std::pair<Mat, Mat> CalculateFourierTransforms(Mat &&img1, Mat &&img2) const;
  Mat CalculateCrossPowerSpectrum(const Mat &dft1, const Mat &dft2) const;
  void ApplyBandpass(Mat &crosspower) const;
  void CalculateFrequencyBandpass();
  Mat CalculateL3(const Mat &crosspower) const;
  void SwapQuadrants(Mat &mat) const;
  std::pair<Point2i, double> GetPeak(const Mat &mat) const;
  Point2f GetPeakSubpixel(const Mat &mat) const;
  Mat CalculateL2(const Mat &L3, const Point2i &L3peak, int L2size) const;
  Mat CalculateL2U(const Mat &L2) const;
  int GetL1size(const Mat &L2U) const;
  Mat CalculateL1(const Mat &L2U, const Point2f &L2Upeak, int L1size) const;
  bool IsOutOfBounds(const Point2f &peak, const Mat &mat, int size) const;
  bool AccuracyReached(const Point2f &L1peak, const Point2f &L1mid) const;
  bool ReduceL2size(int &L2size) const;
};
