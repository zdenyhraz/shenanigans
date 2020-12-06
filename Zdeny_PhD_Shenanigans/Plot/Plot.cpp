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
  auto idx = plots.find(name);
  if (idx != plots.end())
  {
    delete idx->second;
    plots.erase(idx);
  }
};

void Plot::CloseAll()
{
  for (auto& plt : plots)
  {
    delete plt.second;
    plots.erase(plt.first);
  }
}

Plot::Plot1D::Plot1D(const std::string& name) : mName(name)
{
}

void Plot::Plot1D::Plot(const std::vector<double>& x, const std::vector<double>& y)
{
  PlotCore(x, {y}, {});
}

void Plot::Plot1D::Plot(const std::vector<double>& x, const std::vector<std::vector<double>>& ys)
{
  PlotCore(x, ys, {});
}

void Plot::Plot1D::Plot(const std::vector<double>& x, const std::vector<double>& y1, const std::vector<double>& y2)
{
  PlotCore(x, {y1}, {y2});
}

void Plot::Plot1D::Plot(const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s)
{
  PlotCore(x, y1s, y2s);
}

void Plot::Plot1D::Plot(double x, double y)
{
  PlotCore(x, {y}, {});
}

void Plot::Plot1D::Plot(double x, const std::vector<double>& ys)
{
  PlotCore(x, ys, {});
}

void Plot::Plot1D::Plot(double x, double y1, double y2)
{
  PlotCore(x, {y1}, {y2});
}

void Plot::Plot1D::Plot(double x, const std::vector<double>& y1s, const std::vector<double>& y2s)
{
  PlotCore(x, y1s, y2s);
}

void Plot::Plot1D::PlotCore(const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s)
{
  int y1cnt = y1s.size();
  int y2cnt = y2s.size();
  int ycnt = y1cnt + y2cnt;

  if (!mInitialized)
    Initialize(ycnt, y1cnt, y2cnt);

  WindowPlot* windowPlot = plots[mName];
  auto plot = windowPlot->ui.widget;

  for (int i = 0; i < ycnt; i++)
  {
    if (i < y1cnt)
      plot->graph(i)->setData(QVector<double>::fromStdVector(x), QVector<double>::fromStdVector(y1s[i]));
    else
      plot->graph(i)->setData(QVector<double>::fromStdVector(x), QVector<double>::fromStdVector(y2s[i - y1cnt]));
  }

  plot->rescaleAxes();
  plot->replot();
  windowPlot->show();

  if (mSavepath.length() > 0)
    plot->savePng(QString::fromStdString(mSavepath), 0, 0, 3, -1);
}

void Plot::Plot1D::PlotCore(double x, const std::vector<double>& y1s, const std::vector<double>& y2s)
{
  int y1cnt = y1s.size();
  int y2cnt = y2s.size();
  int ycnt = y1cnt + y2cnt;

  if (!mInitialized)
    Initialize(ycnt, y1cnt, y2cnt);

  WindowPlot* windowPlot = plots[mName];
  auto plot = windowPlot->ui.widget;

  for (int i = 0; i < ycnt; i++)
  {
    auto graph = plot->graph(i);

    if (i < y1cnt)
      graph->addData(x, y1s[i]);
    else
      graph->addData(x, y2s[i - y1cnt]);
  }

  plot->rescaleAxes();
  plot->replot();
  windowPlot->show();

  if (mSavepath.length() > 0)
    plot->savePng(QString::fromStdString(mSavepath), 0, 0, 3, -1);
}

void Plot::Plot1D::Initialize(int ycnt, int y1cnt, int y2cnt)
{
  WindowPlot* windowPlot = new WindowPlot(mName, 1.3, OnClose);
  windowPlot->move(GetNewPlotPosition(windowPlot));
  plots[mName] = windowPlot;
  auto plot = windowPlot->ui.widget;

  plot->xAxis->setTickLabelFont(fontTicks);
  plot->yAxis->setTickLabelFont(fontTicks);
  plot->yAxis2->setTickLabelFont(fontTicks);
  plot->xAxis->setLabelFont(fontLabels);
  plot->yAxis->setLabelFont(fontLabels);
  plot->yAxis2->setLabelFont(fontLabels);
  plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
  plot->xAxis->setLabel(QString::fromStdString(mXlabel));
  plot->yAxis->setLabel(QString::fromStdString(mY1label));
  plot->yAxis2->setLabel(QString::fromStdString(mY2label));
  plot->legend->setVisible(mLegend && ycnt > 1);
  plot->legend->setFont(fontLegend);

  switch (mLegendPosition)
  {
  case LegendPosition::TopRight:
    plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignRight);
    break;
  case LegendPosition::BotRight:
    plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom | Qt::AlignRight);
    break;
  case LegendPosition::BotLeft:
    plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom | Qt::AlignLeft);
    break;
  case LegendPosition::TopLeft:
    plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignLeft);
    break;
  }

  if (mY1Log)
  {
    plot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    plot->yAxis->setNumberPrecision(0);
    plot->yAxis->setNumberFormat("eb");
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    plot->yAxis->setTicker(logTicker);
  }

  if (mY2Log)
  {
    plot->yAxis2->setScaleType(QCPAxis::stLogarithmic);
    plot->yAxis2->setNumberPrecision(0);
    plot->yAxis2->setNumberFormat("eb");
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    plot->yAxis2->setTicker(logTicker);
  }

  for (int i = 0; i < ycnt; i++)
  {
    auto graph = plot->graph(i);

    if (i < y1cnt)
    {
      plot->addGraph(plot->xAxis, plot->yAxis);
      if (mY1names.size() > i)
        graph->setName(QString::fromStdString(mY1names[i]));
      else
        graph->setName(QString::fromStdString("y1_" + to_string(i + 1)));
    }
    else
    {
      plot->addGraph(plot->xAxis, plot->yAxis2);
      if (mY2names.size() > i - y1cnt)
        graph->setName(QString::fromStdString(mY2names[i - y1cnt]));
      else
        graph->setName(QString::fromStdString("y2_" + to_string(i - y1cnt + 1)));
    }

    if (i < mPens.size())
      graph->setPen(mPens[i]);
    else
      graph->setPen(QPen(QColor(randr(0, 255), randr(0, 255), randr(0, 255)), pt, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    if (mScatter)
    {
      graph->setLineStyle(QCPGraph::lsNone);
      graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, pt));
    }
  }

  if (y2cnt > 0)
    plot->yAxis2->setVisible(true);

  windowPlot->show();
  QCoreApplication::processEvents();
  mInitialized = true;
}

void Plot::Plot1D::Reset()
{
  auto idx = plots.find(mName);
  if (idx != plots.end())
  {
    LOG_DEBUG("Reseting 1Dplot '{}'", mName);
    WindowPlot* windowPlot = idx->second;
    auto plot = windowPlot->ui.widget;

    for (int i = 0; i < plot->graphCount(); i++)
      plot->graph(i)->data().data()->clear();

    plot->rescaleAxes();
    plot->replot();
    windowPlot->show();
  }
}
