#pragma once
#include "stdafx.h"
#include "Gui/Windows/Plot/WindowPlot.h"

class Plot
{
public:
  static QPoint GetNewPlotPosition(WindowPlot* windowPlot);
  static std::function<void(std::string)> OnClose;
  static void CloseAll();

  static std::map<std::string, WindowPlot*> plots;
  static std::vector<QPen> pens;
  static double pt;

  static QFont fontTicks;
  static QFont fontLabels;
  static QFont fontLegend;

  static QColor black;
  static QColor blue;
  static QColor orange;
  static QColor yellow;
  static QColor magenta;
  static QColor green;
  static QColor cyan;
  static QColor red;

  enum class LegendPosition
  {
    TopRight,
    BotRight,
    BotLeft,
    TopLeft
  };

  class Plot1D
  {
  public:
    Plot1D(const std::string& name);

    void Plot(const std::vector<double>& x, const std::vector<double>& y, bool newplot = true);
    void Plot(const std::vector<double>& x, const std::vector<std::vector<double>>& ys, bool newplot = true);
    void Plot(const std::vector<double>& x, const std::vector<double>& y1, const std::vector<double>& y2, bool newplot = true);
    void Plot(const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s, bool newplot = true);

    void Plot(double x, double y);
    void Plot(double x, const std::vector<double>& ys);
    void Plot(double x, double y1, double y2);
    void Plot(double x, const std::vector<double>& y1s, const std::vector<double>& y2s);

    void Reset();

    std::string mName = "plot";
    std::string mXlabel = "x";
    std::string mY1label = "y";
    std::string mY2label = "y2";
    std::vector<std::string> mY1names = {};
    std::vector<std::string> mY2names = {};
    std::vector<QPen> mPens = pens;
    std::string mSavepath = {};
    bool mLegend = true;
    LegendPosition mLegendPosition = LegendPosition::TopRight;
    bool mY1Log = false;
    bool mY2Log = false;
    bool mScatter = false;
    size_t mCounter = 0;

  private:
    void PlotCore(const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s, bool newplot);
    void PlotCore(double x, const std::vector<double>& y1s, const std::vector<double>& y2s);
    void Initialize(int ycnt, int y1cnt, int y2cnt);
    std::string GetName();

    bool mInitialized = false;
  };

  class Plot2D
  {
  public:
    Plot2D(const std::string& name);

    void Plot(const Mat& z, bool newplot = true);
    void Plot(const std::vector<std::vector<double>>& z, bool newplot = true);

    std::string mName = "plot";
    std::string mXlabel = "x";
    std::string mYlabel = "y";
    std::string mZlabel = "z";
    double mXmin = 0;
    double mXmax = 1;
    double mYmin = 0;
    double mYmax = 1;
    double mColRowRatio = 0;
    std::string mSavepath = {};
    bool mShowAxisLabels = false;
    QCPColorGradient mColormapType = QCPColorGradient::gpJet;
    size_t mCounter = 0;

  private:
    void PlotCore(const std::vector<std::vector<double>>& z, bool newplot);
    void Initialize(int xcnt, int ycnt);
    void Reset();
    std::string GetName();

    bool mInitialized = false;
  };
};
