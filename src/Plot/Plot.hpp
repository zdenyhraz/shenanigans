#pragma once
#include "Application/Windows/Plot/WindowPlot.hpp"

class Plot
{
public:
  static QPoint GetNewPlotPosition(WindowPlot* windowPlot, const std::string& name);
  static std::function<void(std::string)> OnClose;
  static void CloseAll();

  static std::map<std::string, std::unique_ptr<WindowPlot>> plots;
  static QFont fontTicks;
  static QFont fontLabels;
  static QFont fontLegend;
  static std::vector<QPen> pens;
  static double pt;
  static QColor black;
  static QColor blue;
  static QColor orange;
  static QColor yellow;
  static QColor magenta;
  static QColor green;
  static QColor cyan;
  static QColor red;

private:
};
