#include "stdafx.h"
#include "Plot.h"
#include "Gui/Windows/Plot/WindowPlot.h"

std::map<std::string, WindowPlot*> Plot::plots;

QFont Plot::fontTicks("Newyork", 17);
QFont Plot::fontLabels("Newyork", 17);
QFont Plot::fontLegend("Newyork", 17);

double Plot::pt = 3.0;

QColor Plot::black(50, 50, 50);
QColor Plot::blue(0, 113.9850, 188.9550);
QColor Plot::orange(216.750, 82.875, 24.990);
QColor Plot::yellow(236.895, 176.970, 31.875);
QColor Plot::magenta(125.970, 46.920, 141.780);
QColor Plot::green(118.830, 171.870, 47.940);
QColor Plot::cyan(76.755, 189.975, 237.915);
QColor Plot::red(161.925, 19.890, 46.920);

QPen Plot::defaultpen(blue, pt);

std::vector<QPen> Plot::defaultpens{
    QPen(blue, pt), QPen(orange, pt), QPen(green, pt), QPen(magenta, pt), QPen(red, pt), QPen(black, pt), QPen(cyan, pt),
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
