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

  static void Set(const std::string& name) { mCurrentPlot = name; }
  static void Clear(const std::string& name) { GetPlot(name).ClearCore(); }
  static void Plot(const std::string& name, const std::vector<double>& x, const std::vector<double>& y) { GetPlot(name).PlotCore(x, {y}, {}); }
  static void Plot(const std::string& name, const std::vector<double>& x, const std::vector<std::vector<double>>& ys) { GetPlot(name).PlotCore(x, ys); }
  static void Plot(const std::string& name, const std::vector<double>& x, const std::vector<double>& y1, const std::vector<double>& y2) { GetPlot(name).PlotCore(x, {y1}, {y2}); }
  static void Plot(const std::string& name, const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s)
  {
    GetPlot(name).PlotCore(x, y1s, y2s);
  }
  static void Plot(const std::string& name, double x, double y) { GetPlot(name).PlotCore(x, {y}); }
  static void Plot(const std::string& name, double x, const std::vector<double>& ys) { GetPlot(name).PlotCore(x, ys); }
  static void Plot(const std::string& name, double x, double y1, double y2) { GetPlot(name).PlotCore(x, {y1}, {y2}); }
  static void Plot(const std::string& name, double x, const std::vector<double>& y1s, const std::vector<double>& y2s) { GetPlot(name).PlotCore(x, y1s, y2s); }

  // unnamed setters
  static void Clear() { GetPlot().ClearCore(); }
  static void Plot(const std::vector<double>& x, const std::vector<double>& y) { GetPlot().PlotCore(x, {y}, {}); }
  static void Plot(const std::vector<double>& x, const std::vector<std::vector<double>>& ys) { GetPlot().PlotCore(x, ys); }
  static void Plot(const std::vector<double>& x, const std::vector<double>& y1, const std::vector<double>& y2) { GetPlot().PlotCore(x, {y1}, {y2}); }
  static void Plot(const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s) { GetPlot().PlotCore(x, y1s, y2s); }
  static void Plot(double x, double y) { GetPlot().PlotCore(x, {y}); }
  static void Plot(double x, const std::vector<double>& ys) { GetPlot().PlotCore(x, ys); }
  static void Plot(double x, double y1, double y2) { GetPlot().PlotCore(x, {y1}, {y2}); }
  static void Plot(double x, const std::vector<double>& y1s, const std::vector<double>& y2s) { GetPlot().PlotCore(x, y1s, y2s); }
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
  static void SetScatterStyle(bool scatterStyle) { GetPlot().mScatterStyle = scatterStyle; }
  static void SetYmin(double ymin) { GetPlot().mYmin = ymin; }
  static void SetYmax(double ymax) { GetPlot().mYmax = ymax; }
  static void SetY2min(double y2min) { GetPlot().mY2min = y2min; }
  static void SetY2max(double y2max) { GetPlot().mY2max = y2max; }

  explicit Plot1D(const std::string& name);

private:
  static Plot1D& GetPlot(const std::string& name = mCurrentPlot);
  static std::unordered_map<std::string, Plot1D> mPlots;
  static std::string mCurrentPlot;

  void PlotCore(const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s = {});
  void PlotCore(double x, const std::vector<double>& y1s, const std::vector<double>& y2s = {});
  void Initialize(int ycnt, int y1cnt, int y2cnt, bool clear);
  void ClearCore();

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
  double mYmin = std::numeric_limits<double>::lowest();
  double mYmax = std::numeric_limits<double>::max();
  double mY2min = std::numeric_limits<double>::lowest();
  double mY2max = std::numeric_limits<double>::max();
};
