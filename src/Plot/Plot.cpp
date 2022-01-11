#include "Log/Logger.h"
#include "Plot.h"

std::map<std::string, std::unique_ptr<WindowPlot>> Plot::plots;

QFont Plot::fontTicks("Newyork", 15);
QFont Plot::fontLabels("Newyork", 17);
QFont Plot::fontLegend("Newyork", 13);
double Plot::pt = 3.0;
QColor Plot::black(50, 50, 50);
QColor Plot::blue(0, 113.9850, 188.9550);
QColor Plot::orange(216.750, 82.875, 24.990);
QColor Plot::yellow(236.895, 176.970, 31.875);
QColor Plot::magenta(125.970, 46.920, 141.780);
QColor Plot::green(118.830, 171.870, 47.940);
QColor Plot::cyan(76.755, 189.975, 237.915);
QColor Plot::red(161.925, 19.890, 46.920);

std::vector<QPen> Plot::pens{
    QPen(blue, pt),
    QPen(orange, pt),
    QPen(green, pt),
    QPen(magenta, pt),
    QPen(red, pt),
    QPen(black, pt),
    QPen(cyan, pt),
};

QPoint Plot::GetNewPlotPosition(WindowPlot* windowPlot, const std::string& name)
{
  int w = 0;
  int h = 0;

  for (const auto& [plotname, plot] : plots)
  {
    if (plotname == name)
      continue;

    if (w + plot->width() > QApplication::desktop()->width())
    {
      w = plot->width();
      h += plot->height() + 25;
      if (h > QApplication::desktop()->height())
        h = 0;
    }
    else
    {
      w += plot->width();
    }
  }

  if (w + windowPlot->width() > QApplication::desktop()->width())
  {
    w = 0;
    h += windowPlot->height() + 25;
    if (h > QApplication::desktop()->height())
      h = 0;
  }

  LOG_TRACE("New plot position = [{},{}], plot is in {}. place", w, h, plots.size());
  return QPoint(w, h);
}

std::function<void(std::string)> Plot::OnClose = [](std::string name)
{
  auto idx = plots.find(name);
  if (idx != plots.end())
    plots.erase(idx);
};

void Plot::CloseAll()
{
  for (auto& plt : plots)
    plots.erase(plt.first);
}
