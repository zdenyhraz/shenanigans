#pragma once
#include "stdafx.h"
#include "Gui/Windows/Plot/WindowPlot.h"

class Plot
{
public:
  static std::map<std::string, WindowPlot*> plots;
  static QFont fontTicks;
  static QFont fontLabels;
  static QFont fontLegend;
  static double pt;
  static QPoint GetNewPlotPosition(WindowPlot* windowPlot);
  static std::vector<QPen> defaultpens;
  static QPen defaultpen;

  // matlab plot colors
  static QColor black;
  static QColor blue;
  static QColor orange;
  static QColor yellow;
  static QColor magenta;
  static QColor green;
  static QColor cyan;
  static QColor red;
};
