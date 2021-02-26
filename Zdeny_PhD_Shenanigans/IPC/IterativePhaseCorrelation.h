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

  IterativePhaseCorrelation(int rows, int cols = 0, double bandpassL = 1.0, double bandpassH = 0.01);
  IterativePhaseCorrelation(const Mat& img, double bandpassL = 1.0, double bandpassH = 0.01);

  void SetSize(int rows, int cols = -1);
  void SetSize(Size size);
  void SetBandpassParameters(double bandpassL, double bandpassH);
  void SetL2size(int L2size) { mL2size = L2size % 2 ? L2size : L2size + 1; }
  void SetL1ratio(double L1ratio) { mL1ratio = L1ratio; }
  void SetUpsampleCoeff(int upsampleCoeff) { mUpsampleCoeff = upsampleCoeff % 2 ? upsampleCoeff : upsampleCoeff + 1; }
  void SetMaxIterations(int maxIterations) { mMaxIterations = maxIterations; }
  void SetInterpolationType(InterpolationType interpolationType) { mInterpolationType = interpolationType; }
  void SetBandpassType(BandpassType type) { mBandpassType = type; }
  void SetWindowType(WindowType type) { mWindowType = type; }
  void SetDebugMode(bool mode) const { mDebugMode = mode; }
  void SetDebugDirectory(const std::string& dir) { mDebugDirectory = dir; }

  int GetRows() const { return mRows; }
  int GetCols() const { return mCols; }
  int GetSize() const { return mRows; }
  double GetBandpassL() const { return mBandpassL; }
  double GetBandpassH() const { return mBandpassH; }
  int GetL2size() const { return mL2size; }
  double GetL1ratio() const { return mL1ratio; }
  int GetUpsampleCoeff() const { return mUpsampleCoeff; }
  Mat GetWindow() const { return mWindow; }
  Mat GetBandpass() const { return mBandpass; }

  Point2f Calculate(const Mat& image1, const Mat& image2) const;
  Point2f Calculate(Mat&& image1, Mat&& image2) const;

  void ShowDebugStuff() const;
  void Optimize(const std::string& trainingImagesDirectory, const std::string& validationImagesDirectory, float maxShift = 2.0, float noiseStdev = 0.01, int itersPerImage = 100,
                double validationRatio = 0.2, int populationSize = ParameterCount * 7);
  void PlotObjectiveFunctionLandscape(const std::string& trainingImagesDirectory, const std::string& validationImagesDirectory, float maxShift, float noiseStdev, int itersPerImage, int iters) const;

private:
  int mRows = 0;
  int mCols = 0;
  double mBandpassL = 0.0;
  double mBandpassH = 0.6;
  int mL2size = 11;
  double mL1ratio = 0.35;
  double mL1ratioStep = 0.05;
  int mUpsampleCoeff = 51;
  int mMaxIterations = 20;
  mutable bool mDebugMode = false;
  BandpassType mBandpassType = BandpassType::Gaussian;
  InterpolationType mInterpolationType = InterpolationType::Linear;
  WindowType mWindowType = WindowType::Hann;
  std::string mDebugDirectory = "Debug";
  mutable AccuracyType mAccuracyType = AccuracyType::SubpixelIterative;
  Mat mBandpass;
  Mat mFrequencyBandpass;
  Mat mWindow;

  void UpdateWindow();
  void UpdateBandpass();
  float LowpassEquation(int row, int col) const;
  float HighpassEquation(int row, int col) const;
  float BandpassGEquation(int row, int col) const;
  float BandpassREquation(int row, int col) const;
  bool IsValid(const Mat& image1, const Mat& image2) const;
  bool CheckSize(const Mat& image1, const Mat& image2) const;
  bool CheckChannels(const Mat& image1, const Mat& image2) const;
  void ConvertToUnitFloat(Mat& image) const;
  void ApplyWindow(Mat& image) const;
  Mat CalculateFourierTransform(Mat&& image) const;
  Mat CalculateCrossPowerSpectrum(const Mat& dft1, const Mat& dft2) const;
  void ApplyBandpass(Mat& crosspower) const;
  void CalculateFrequencyBandpass();
  Mat CalculateL3(Mat&& crosspower) const;
  Point2f GetPeak(const Mat& mat) const;
  Point2f GetPeakSubpixel(const Mat& mat) const;
  Mat CalculateL2(const Mat& L3, const Point2f& L3peak, int L2size) const;
  Mat CalculateL2U(const Mat& L2) const;
  int GetL1size(const Mat& L2U, double L1ratio) const;
  Mat CalculateL1(const Mat& L2U, const Point2f& L2Upeak, int L1size) const;
  bool IsOutOfBounds(const Point2f& peak, const Mat& mat, int size) const;
  bool AccuracyReached(const Point2f& L1peak, const Point2f& L1mid) const;
  bool ReduceL2size(int& L2size) const;
  void ReduceL1ratio(double& L1ratio) const;
  void InitializePlots() const;

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

  std::vector<Mat> LoadImages(const std::string& imagesDirectory) const;
  std::vector<std::tuple<Mat, Mat, Point2f>> CreateImagePairs(const std::vector<Mat>& images, double maxShiftRatio, int itersPerImage, double noiseStdev) const;
  void AddNoise(Mat& image, double noiseStdev) const;
  const std::function<double(const std::vector<double>&)> CreateObjectiveFunction(const std::vector<std::tuple<Mat, Mat, Point2f>>& imagePairs) const;
  std::vector<double> CalculateOptimalParameters(const std::function<double(const std::vector<double>&)>& obj, const std::function<double(const std::vector<double>&)>& valid,
                                                 int populationSize) const;
  void ApplyOptimalParameters(const std::vector<double>& optimalParameters);
  std::string BandpassType2String(BandpassType type, double bandpassL, double bandpassH) const;
  std::string WindowType2String(WindowType type) const;
  std::string InterpolationType2String(InterpolationType type) const;
  void ShowOptimizationPlots(const std::vector<Point2f>& shiftsReference, const std::vector<Point2f>& shiftsPixel, const std::vector<Point2f>& shiftsNonit, const std::vector<Point2f>& shiftsBefore,
                             const std::vector<Point2f>& shiftsAfter) const;
  std::vector<Point2f> GetShifts(const std::vector<std::tuple<Mat, Mat, Point2f>>& imagePairs) const;
  std::vector<Point2f> GetNonIterativeShifts(const std::vector<std::tuple<Mat, Mat, Point2f>>& imagePairs) const;
  std::vector<Point2f> GetPixelShifts(const std::vector<std::tuple<Mat, Mat, Point2f>>& imagePairs) const;
  std::vector<Point2f> GetReferenceShifts(const std::vector<std::tuple<Mat, Mat, Point2f>>& imagePairs) const;
  double GetAverageAccuracy(const std::vector<Point2f>& shiftsReference, const std::vector<Point2f>& shifts) const;
  static double GetFractionalPart(double x);
  void PlotObjectiveFunctionLandscape(const std::function<double(const std::vector<double>&)>& obj, int iters) const;
};
