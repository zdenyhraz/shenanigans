#pragma once
#include "stdafx.h"
#include "Gui/Windows/Plot/WindowPlot.h"

class Plot
{
public:
  static std::map<std::string, WindowPlot *> plots;
  static QFont fontTicks;
  static QFont fontLabels;
  static double pt;
  static QPoint GetNewPlotPosition(WindowPlot *windowPlot);
  static std::vector<QPen> defaultpens;
  static QPen defaultpen;

  // plot colors
  static QColor blue;
  static QColor black;
  static QColor red;
  static QColor orange;
  static QColor cyan;
  static QColor magenta;
  static QColor green;

  // matlab plot colors
  static QColor matlabBlue;
  static QColor matlabOrange;
  static QColor matlabYellow;
  static QColor matlabMagenta;
  static QColor matlabGreen;
  static QColor matlabCyan;
  static QColor matlabRed;
};
