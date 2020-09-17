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
  IterativePhaseCorrelation(int rows, int cols, double bandpassL, double bandpassH) : mRows(rows), mCols(cols), mBandpassL(bandpassL), mBandpassH(bandpassH)
  {
    mWindow = edgemask(mRows, mCols);
    mBandpass = bandpassian(mRows, mCols, mBandpassL, mBandpassH);
    CalculateFrequencyBandpass();
  }

  void SetBandpassParameters(double bandpassL, double bandpassH)
  {
    mBandpassL = bandpassL;
    mBandpassH = bandpassH;
    mBandpass = bandpassian(mRows, mCols, mBandpassL, mBandpassH);
    CalculateFrequencyBandpass();
  }

  void SetSize(int rows, int cols = -1)
  {
    mRows = rows;
    mCols = cols > 0 ? cols : rows;
    mWindow = edgemask(mRows, mCols);
    mBandpass = bandpassian(mRows, mCols, mBandpassL, mBandpassH);
    CalculateFrequencyBandpass();
  }

  void SetL2size(int L2size) { mL2size = L2size % 2 ? L2size : L2size + 1; }
  void SetL1ratio(double L1ratio) { mL1ratio = L1ratio; }
  void SetUpsampleCoeff(int UpsampleCoeff) { mUpsampleCoeff = UpsampleCoeff; }
  void SetDivisionEpsilon(double DivisionEpsilon) { mDivisionEpsilon = DivisionEpsilon; }
  void SetMaxIterations(int MaxIterations) { mMaxIterations = MaxIterations; }
  void SetInterpolate(bool Interpolate) { mInterpolate = Interpolate; }
  void SetInterpolationType() {}
  void SetApplyWindow(bool ApplyWindow) { mApplyWindow = ApplyWindow; }
  void SetApplyBandpass(bool ApplyBandpass) { mApplyBandpass = ApplyBandpass; }
  void SetSubpixelExtimation(bool SubpixelEstimation) { mSubpixelEstimation = SubpixelEstimation; }
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

  inline cv::Point2f Calculate(const Mat &image1, const Mat &image2);
  inline cv::Point2f Calculate(Mat &&image1, Mat &&image2);

private:
  int mRows = 0;
  int mCols = 0;
  double mBandpassL = 1;
  double mBandpassH = 200;
  int mL2size = 15;
  double mL1ratio = 0.35;
  int mUpsampleCoeff = 51;
  double mDivisionEpsilon = 0;
  int mMaxIterations = 20;
  bool mInterpolate = true;
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

  void ConvertToUnitFloat(Mat &img1, Mat &img2);
  void ApplyWindow(Mat &img1, Mat &img2);
  std::pair<Mat, Mat> CalculateFourierTransforms(Mat &&img1, Mat &&img2);
  Mat CalculateCrossPowerSpectrum(const Mat &dft1, const Mat &dft2);
  void ApplyBandpass(Mat &crosspower);
  void CalculateFrequencyBandpass();
  Mat CalculateL3(const Mat &crosspower);
  void SwapQuadrants(Mat &mat);
  std::pair<Point2i, double> GetPeak(const Mat &mat);
};
