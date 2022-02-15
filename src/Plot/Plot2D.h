#pragma once
#include "Plot.h"

class Plot2D
{
public:
  static void Set(const std::string& name) { mCurrentPlot = name; }
  static void Plot(const std::string& name, const cv::Mat& z) { GetPlot(name).PlotCore(z); }
  static void Plot(const std::string& name, const std::vector<std::vector<double>>& z) { GetPlot(name).PlotCore(z); }

  // unnamed setters
  static void Plot(const cv::Mat& z) { GetPlot().PlotCore(z); }
  static void Plot(const std::vector<std::vector<double>>& z) { GetPlot().PlotCore(z); }
  static void SetXlabel(const std::string& xlabel) { GetPlot().mXlabel = xlabel; }
  static void SetYlabel(const std::string& ylabel) { GetPlot().mYlabel = ylabel; }
  static void SetZlabel(const std::string& zlabel) { GetPlot().mZlabel = zlabel; }
  static void SetXmin(double xmin) { GetPlot().mXmin = xmin; }
  static void SetXmax(double xmax) { GetPlot().mXmax = xmax; }
  static void SetYmin(double ymin) { GetPlot().mYmin = ymin; }
  static void SetYmax(double ymax) { GetPlot().mYmax = ymax; }
  static void SetColRowRatio(double colRowRatio) { GetPlot().mColRowRatio = colRowRatio; }
  static void SetSavePath(const std::string& savePath) { GetPlot().mSavepath = savePath; }
  static void SetColorMapType(QCPColorGradient colorMapType) { GetPlot().mColormapType = colorMapType; }
  static void ShowAxisLabels(bool showAxisLabels) { GetPlot().mShowAxisLabels = showAxisLabels; }

private:
  static Plot2D& GetPlot(const std::string& name = mCurrentPlot);
  static std::map<std::string, Plot2D> mPlots;
  static std::string mCurrentPlot;

  explicit Plot2D(const std::string& name);
  void PlotCore(const cv::Mat& z);
  void PlotCore(const std::vector<std::vector<double>>& z);
  void Initialize(int xcnt, int ycnt);
  void ClearCore();

  std::string mName = "plot";
  std::string mXlabel = "x";
  std::string mYlabel = "y";
  std::string mZlabel = "z";
  double mXmin = 0;
  double mXmax = 1;
  double mYmin = 0;
  double mYmax = 1;
  double mColRowRatio = 0;
  std::string mSavepath;
  bool mShowAxisLabels = false;
  QCPColorGradient mColormapType = QCPColorGradient::gpJet;
};
