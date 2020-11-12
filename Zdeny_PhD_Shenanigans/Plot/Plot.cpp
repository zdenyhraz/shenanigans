#include "stdafx.h"
#include "Plot.h"
#include "Gui/Windows/Plot/WindowPlot.h"

std::map<std::string, WindowPlot*> Plot::plots;

QFont Plot::fontTicks("Newyork", 17);
QFont Plot::fontLabels("Newyork", 17);
QFont Plot::fontLegend("Newyork", 17);

double Plot::pt = 3.0;

QColor Plot::blue(30, 80, 255);
QColor Plot::red(220, 20, 60);
QColor Plot::green(119, 182, 48);
QColor Plot::black(50, 50, 50);
QColor Plot::orange(255, 165, 0);
QColor Plot::cyan(64, 224, 208);
QColor Plot::magenta(150, 0, 150);

QColor Plot::matlabBlue(0, 113.9850, 188.9550);
QColor Plot::matlabOrange(216.750, 82.875, 24.990);
QColor Plot::matlabYellow(236.895, 176.970, 31.875);
QColor Plot::matlabMagenta(125.970, 46.920, 141.780);
QColor Plot::matlabGreen(118.830, 171.870, 47.940);
QColor Plot::matlabCyan(76.755, 189.975, 237.915);
QColor Plot::matlabRed(161.925, 19.890, 46.920);

QPen Plot::defaultpen(matlabBlue, pt);

std::vector<QPen> Plot::defaultpens{
    QPen(matlabBlue, pt), QPen(matlabOrange, pt), QPen(matlabGreen, pt), QPen(matlabMagenta, pt),
    QPen(matlabRed, pt),  QPen(black, pt),        QPen(matlabCyan, pt),
};

QPoint Plot::GetNewPlotPosition(WindowPlot* windowPlot)
{
  int w = 0;
  int h = 0;
  for (auto& plot : plots)
  {
    if (w + plot.second->width() > QApplication::desktop()->width())
    {
      w = plot.second->width();
      h += plot.second->height() + 25;
      if (h > QApplication::desktop()->height())
        h = 0;
    }
    else
    {
      w += plot.second->width();
    }
  }

  if (w + windowPlot->width() > QApplication::desktop()->width())
  {
    w = 0;
    h += windowPlot->height() + 25;
    if (h > QApplication::desktop()->height())
      h = 0;
  }

  return QPoint(w, h);
}
