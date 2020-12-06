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

std::vector<QPen> Plot::pens{
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

std::function<void(std::string)> Plot::OnClose = [](std::string name) {
  auto idx = Plot::plots.find(name);
  if (idx != Plot::plots.end())
  {
    delete idx->second;
    Plot::plots.erase(idx);
  }
};

void Plot::CloseAll()
{
  for (auto& plt : Plot::plots)
  {
    delete plt.second;
    Plot::plots.erase(plt.first);
  }
}

Plot::Plot1D::Plot1D(const std::string& name) : mName(name)
{
}

void Plot::Plot1D::Plot(const std::vector<double>& x, const std::vector<double>& y)
{
  PlotCoreReplot(x, {y}, {});
}

void Plot::Plot1D::Plot(const std::vector<double>& x, const std::vector<std::vector<double>>& ys)
{
  PlotCoreReplot(x, ys, {});
}

void Plot::Plot1D::Plot(const std::vector<double>& x, const std::vector<double>& y1, const std::vector<double>& y2)
{
  PlotCoreReplot(x, {y1}, {y2});
}

void Plot::Plot1D::Plot(const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s)
{
  PlotCoreReplot(x, y1s, y2s);
}

void Plot::Plot1D::Plot(double x, double y)
{
  PlotCoreAdd(x, {y}, {});
}

void Plot::Plot1D::Plot(double x, const std::vector<double>& ys)
{
  PlotCoreAdd(x, ys, {});
}

void Plot::Plot1D::Plot(double x, double y1, double y2)
{
  PlotCoreAdd(x, {y1}, {y2});
}

void Plot::Plot1D::Plot(double x, const std::vector<double>& y1s, const std::vector<double>& y2s)
{
  PlotCoreAdd(x, y1s, y2s);
}

void Plot::Plot1D::PlotCoreReplot(const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s)
{
  int y1cnt = y1s.size();
  int y2cnt = y2s.size();
  int ycnt = y1cnt + y2cnt;

  if (!mInitialized)
    Initialize(ycnt, y1cnt, y2cnt);

  WindowPlot* windowPlot = Plot::plots[mName];

  for (int i = 0; i < ycnt; i++)
  {
    if (i < y1cnt)
      windowPlot->ui.widget->graph(i)->setData(QVector<double>::fromStdVector(x), QVector<double>::fromStdVector(y1s[i]));
    else
      windowPlot->ui.widget->graph(i)->setData(QVector<double>::fromStdVector(x), QVector<double>::fromStdVector(y2s[i - y1cnt]));
  }

  windowPlot->ui.widget->rescaleAxes();
  windowPlot->ui.widget->replot();
  windowPlot->show();

  if (mSavepath != std::string{})
    windowPlot->ui.widget->savePng(QString::fromStdString(mSavepath), 0, 0, 3, -1);
}

void Plot::Plot1D::PlotCoreAdd(double x, const std::vector<double>& y1s, const std::vector<double>& y2s)
{
}

void Plot::Plot1D::Initialize(int ycnt, int y1cnt, int y2cnt)
{
  WindowPlot* windowPlot = new WindowPlot(mName, 1.3, OnClose);
  windowPlot->move(Plot::GetNewPlotPosition(windowPlot));
  Plot::plots[mName] = windowPlot;

  windowPlot->ui.widget->xAxis->setTickLabelFont(Plot::fontTicks);
  windowPlot->ui.widget->yAxis->setTickLabelFont(Plot::fontTicks);
  windowPlot->ui.widget->yAxis2->setTickLabelFont(Plot::fontTicks);
  windowPlot->ui.widget->xAxis->setLabelFont(Plot::fontLabels);
  windowPlot->ui.widget->yAxis->setLabelFont(Plot::fontLabels);
  windowPlot->ui.widget->yAxis2->setLabelFont(Plot::fontLabels);
  windowPlot->ui.widget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
  windowPlot->ui.widget->xAxis->setLabel(QString::fromStdString(mXlabel));
  windowPlot->ui.widget->yAxis->setLabel(QString::fromStdString(mY1label));
  windowPlot->ui.widget->yAxis2->setLabel(QString::fromStdString(mY2label));
  windowPlot->ui.widget->legend->setVisible(mLegend && ycnt > 1);
  windowPlot->ui.widget->legend->setFont(Plot::fontLegend);

  switch (mLegendPosition)
  {
  case LegendPosition::TopRight:
    windowPlot->ui.widget->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignRight);
    break;
  case LegendPosition::BotRight:
    windowPlot->ui.widget->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom | Qt::AlignRight);
    break;
  case LegendPosition::BotLeft:
    windowPlot->ui.widget->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom | Qt::AlignLeft);
    break;
  case LegendPosition::TopLeft:
    windowPlot->ui.widget->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignLeft);
    break;
  }

  if (mY1Log)
  {
    windowPlot->ui.widget->yAxis->setScaleType(QCPAxis::stLogarithmic);
    windowPlot->ui.widget->yAxis->setNumberPrecision(0);
    windowPlot->ui.widget->yAxis->setNumberFormat("eb");
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    windowPlot->ui.widget->yAxis->setTicker(logTicker);
  }

  if (mY2Log)
  {
    windowPlot->ui.widget->yAxis2->setScaleType(QCPAxis::stLogarithmic);
    windowPlot->ui.widget->yAxis2->setNumberPrecision(0);
    windowPlot->ui.widget->yAxis2->setNumberFormat("eb");
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    windowPlot->ui.widget->yAxis2->setTicker(logTicker);
  }

  for (int i = 0; i < ycnt; i++)
  {
    if (i < y1cnt)
    {
      windowPlot->ui.widget->addGraph(windowPlot->ui.widget->xAxis, windowPlot->ui.widget->yAxis);
      if (mY1names.size() > i)
        windowPlot->ui.widget->graph(i)->setName(QString::fromStdString(mY1names[i]));
      else
        windowPlot->ui.widget->graph(i)->setName(QString::fromStdString("y1_" + to_string(i + 1)));
    }
    else
    {
      windowPlot->ui.widget->addGraph(windowPlot->ui.widget->xAxis, windowPlot->ui.widget->yAxis2);
      if (mY2names.size() > i - y1cnt)
        windowPlot->ui.widget->graph(i)->setName(QString::fromStdString(mY2names[i - y1cnt]));
      else
        windowPlot->ui.widget->graph(i)->setName(QString::fromStdString("y2_" + to_string(i - y1cnt + 1)));
    }

    if (i < pens.size())
      windowPlot->ui.widget->graph(i)->setPen(pens[i]);
    else
      windowPlot->ui.widget->graph(i)->setPen(QPen(QColor(randr(0, 255), randr(0, 255), randr(0, 255)), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  }

  if (y2cnt > 0)
    windowPlot->ui.widget->yAxis2->setVisible(true);

  windowPlot->show();
  QCoreApplication::processEvents();
  mInitialized = true;
}
