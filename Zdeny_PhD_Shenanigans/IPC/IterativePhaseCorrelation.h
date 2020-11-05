#pragma once
#include "stdafx.h"

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
    Hamming,
  };

  IterativePhaseCorrelation(int rows, int cols, double bandpassL = 1.0, double bandpassH = 0.01);

  // setters
  void SetSize(int rows, int cols = -1);
  void SetBandpassParameters(double bandpassL, double bandpassH);
  void SetL2size(int L2size) { mL2size = L2size % 2 ? L2size : L2size + 1; }
  void SetL1ratio(double L1ratio) { mL1ratio = L1ratio; }
  void SetUpsampleCoeff(int upsampleCoeff) { mUpsampleCoeff = upsampleCoeff % 2 ? upsampleCoeff : upsampleCoeff + 1; }
  void SetMaxIterations(int maxIterations) { mMaxIterations = maxIterations; }
  void SetInterpolationType(InterpolationFlags interpolationType) { mInterpolationType = interpolationType; }
  void SetBandpassType(BandpassType type) { mBandpassType = type; }
  void SetWindowType(WindowType type) { mWindowType = type; }

  // getters
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

  // main calc methods
  Point2f Calculate(const Mat &image1, const Mat &image2) const; // does not mangle the source images
  Point2f Calculate(Mat &&image1, Mat &&image2) const;           // mangles the source images
  void Optimize(const std::string &trainingImagesDirectory, const std::string &validationImagesDirectory, float maxShiftRatio = 0.25,
      float noiseStdev = 0.1, int itersPerImage = 5);
  void ShowDebugStuff() const;

private:
  int mRows = 0;
  int mCols = 0;
  double mBandpassL = 1.0;
  double mBandpassH = 0.01;
  int mL2size = 15;
  double mL1ratio = 0.35;
  int mUpsampleCoeff = 51;
  int mMaxIterations = 20;
  InterpolationFlags mInterpolationType = INTER_LINEAR;
  Mat mBandpass;
  Mat mFrequencyBandpass;
  Mat mWindow;
  bool mSave = false;
  string mSavedir = "";
  cv::Size mSavesize = cv::Size(500, 500);
  int mSavecntr = 0;
  BandpassType mBandpassType = BandpassType::Gaussian;
  WindowType mWindowType = WindowType::Hann;

  // internal methods
  void UpdateWindow();
  void UpdateBandpass();
  float BandpassLEquation(int row, int col) const;
  float BandpassHEquation(int row, int col) const;
  float BandpassGEquation(int row, int col) const;
  float BandpassREquation(int row, int col) const;
  bool IsValid(const Mat &img1, const Mat &img2) const;
  bool CheckSize(const Mat &img1, const Mat &img2) const;
  bool CheckChannels(const Mat &img1, const Mat &img2) const;
  void ConvertToUnitFloat(Mat &img1, Mat &img2) const;
  void ApplyWindow(Mat &img1, Mat &img2) const;
  std::pair<Mat, Mat> CalculateFourierTransforms(Mat &img1, Mat &img2) const;
  Mat CalculateCrossPowerSpectrum(const Mat &dft1, const Mat &dft2) const;
  void ApplyBandpass(Mat &crosspower) const;
  void CalculateFrequencyBandpass();
  Mat CalculateL3(const Mat &crosspower) const;
  void SwapQuadrants(Mat &mat) const;
  Point2f GetPeak(const Mat &mat) const;
  Point2f GetPeakSubpixel(const Mat &mat) const;
  Mat CalculateL2(const Mat &L3, const Point2f &L3peak, int L2size) const;
  Mat CalculateL2U(const Mat &L2) const;
  int GetL1size(const Mat &L2U) const;
  Mat CalculateL1(const Mat &L2U, const Point2f &L2Upeak, int L1size) const;
  bool IsOutOfBounds(const Point2f &peak, const Mat &mat, int size) const;
  bool AccuracyReached(const Point2f &L1peak, const Point2f &L1mid) const;
  bool ReduceL2size(int &L2size) const;
};
