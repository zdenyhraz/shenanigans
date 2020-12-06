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

    void Plot(const std::vector<double>& x, const std::vector<double>& y);
    void Plot(const std::vector<double>& x, const std::vector<std::vector<double>>& ys);
    void Plot(const std::vector<double>& x, const std::vector<double>& y1, const std::vector<double>& y2);
    void Plot(const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s);

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

  private:
    void PlotCore(const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s);
    void PlotCore(double x, const std::vector<double>& y1s, const std::vector<double>& y2s);
    void Initialize(int ycnt, int y1cnt, int y2cnt);

    bool mInitialized = false;
  };

  class Plot2D
  {
  public:
    void Plot();
    // TODO
  private:
  };
};
