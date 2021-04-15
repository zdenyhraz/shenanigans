#pragma once
#include "Plot.h"

class Plot1D
{
public:
  enum class LegendPosition
  {
    TopRight,
    BotRight,
    BotLeft,
    TopLeft
  };

  static void Plot(const std::string& name, const std::vector<double>& x, const std::vector<double>& y, bool newplot = false) { GetPlot(name).Plot(x, y, newplot); }
  static void Plot(const std::string& name, const std::vector<double>& x, const std::vector<std::vector<double>>& ys, bool newplot = false) { GetPlot(name).Plot(x, ys, newplot); }
  static void Plot(const std::string& name, const std::vector<double>& x, const std::vector<double>& y1, const std::vector<double>& y2, bool newplot = false)
  {
    GetPlot(name).Plot(x, y1, y2, newplot);
  }
  static void Plot(const std::string& name, const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s, bool newplot = false)
  {
    GetPlot(name).Plot(x, y1s, y2s, newplot);
  }
  static void Plot(const std::string& name, double x, double y) { GetPlot(name).Plot(x, y); }
  static void Plot(const std::string& name, double x, const std::vector<double>& ys) { GetPlot(name).Plot(x, ys); }
  static void Plot(const std::string& name, double x, double y1, double y2) { GetPlot(name).Plot(x, y1, y2); }
  static void Plot(const std::string& name, double x, const std::vector<double>& y1s, const std::vector<double>& y2s) { GetPlot(name).Plot(x, y1s, y2s); }
  static void Reset(const std::string& name) { GetPlot(name).Reset(); }

  // named setters
  static void SetXlabel(const std::string& name, const std::string& xlabel) { GetPlot(name).mXlabel = xlabel; }
  static void SetYlabel(const std::string& name, const std::string& ylabel) { GetPlot(name).mYlabel = ylabel; }
  static void SetY2label(const std::string& name, const std::string& y2label) { GetPlot(name).mY2label = y2label; }
  static void SetYnames(const std::string& name, const std::vector<std::string>& ynames) { GetPlot(name).mYnames = ynames; }
  static void SetY2names(const std::string& name, const std::vector<std::string>& y2names) { GetPlot(name).mY2names = y2names; }
  static void SetPens(const std::string& name, const std::vector<QPen>& pens) { GetPlot(name).mPens = pens; }
  static void SetSavePath(const std::string& name, const std::string& savePath) { GetPlot(name).mSavepath = savePath; }
  static void SetLegendVisible(const std::string& name, bool legendVisible) { GetPlot(name).mLegendVisible = legendVisible; }
  static void SetLegendPosition(const std::string& name, LegendPosition legendPosition) { GetPlot(name).mLegendPosition = legendPosition; }
  static void SetYLogarithmic(const std::string& name, bool yLogarithmic) { GetPlot(name).mYLogarithmic = yLogarithmic; }
  static void SetY2Logarithmic(const std::string& name, bool y2Logarithmic) { GetPlot(name).mY2Logarithmic = y2Logarithmic; }
  static void SetScatterStyle(const std::string& name, bool scatterStype) { GetPlot(name).mScatterStyle = scatterStype; }

  // unnamed setters
  static void SetXlabel(const std::string& xlabel) { GetPlot().mXlabel = xlabel; }
  static void SetYlabel(const std::string& ylabel) { GetPlot().mYlabel = ylabel; }
  static void SetY2label(const std::string& y2label) { GetPlot().mY2label = y2label; }
  static void SetYnames(const std::vector<std::string>& ynames) { GetPlot().mYnames = ynames; }
  static void SetY2names(const std::vector<std::string>& y2names) { GetPlot().mY2names = y2names; }
  static void SetPens(const std::vector<QPen>& pens) { GetPlot().mPens = pens; }
  static void SetSavePath(const std::string& savePath) { GetPlot().mSavepath = savePath; }
  static void SetLegendVisible(bool legendVisible) { GetPlot().mLegendVisible = legendVisible; }
  static void SetLegendPosition(LegendPosition legendPosition) { GetPlot().mLegendPosition = legendPosition; }
  static void SetYLogarithmic(bool yLogarithmic) { GetPlot().mYLogarithmic = yLogarithmic; }
  static void SetY2Logarithmic(bool y2Logarithmic) { GetPlot().mY2Logarithmic = y2Logarithmic; }
  static void SetScatterStyle(bool scatterStype) { GetPlot().mScatterStyle = scatterStype; }

private:
  Plot1D(const std::string& name);
  void Plot(const std::vector<double>& x, const std::vector<double>& y, bool newplot);
  void Plot(const std::vector<double>& x, const std::vector<std::vector<double>>& ys, bool newplot);
  void Plot(const std::vector<double>& x, const std::vector<double>& y1, const std::vector<double>& y2, bool newplot);
  void Plot(const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s, bool newplot);
  void Plot(double x, double y);
  void Plot(double x, const std::vector<double>& ys);
  void Plot(double x, double y1, double y2);
  void Plot(double x, const std::vector<double>& y1s, const std::vector<double>& y2s);
  void Reset();
  void PlotCore(const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s, bool newplot);
  void PlotCore(double x, const std::vector<double>& y1s, const std::vector<double>& y2s);
  void Initialize(int ycnt, int y1cnt, int y2cnt);
  std::string GetName();
  static Plot1D& GetPlot(const std::string& name = mLastAccessedPlot);

  static std::unordered_map<std::string, Plot1D> mPlots;
  static std::string mLastAccessedPlot;
  std::string mName = "plot";
  std::string mXlabel = "x";
  std::string mYlabel = "y";
  std::string mY2label = "y2";
  std::vector<std::string> mYnames = {};
  std::vector<std::string> mY2names = {};
  std::vector<QPen> mPens = Plot::pens;
  std::string mSavepath = {};
  bool mLegendVisible = true;
  LegendPosition mLegendPosition = LegendPosition::TopRight;
  bool mYLogarithmic = false;
  bool mY2Logarithmic = false;
  bool mScatterStyle = false;
  size_t mCounter = 0;
  bool mInitialized = false;
};
