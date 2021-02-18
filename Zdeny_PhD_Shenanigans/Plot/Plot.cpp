#include "stdafx.h"
#include "Plot.h"
#include "Gui/Windows/Plot/WindowPlot.h"

std::map<std::string, std::unique_ptr<WindowPlot>> Plot::plots;
std::unordered_map<std::string, Plot::Plot1D> Plot::mPlots1D;
std::unordered_map<std::string, Plot::Plot2D> Plot::mPlots2D;

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

std::function<void(std::string)> Plot::OnClose = [](std::string name) {
  auto idx = plots.find(name);
  if (idx != plots.end())
    plots.erase(idx);
};

void Plot::CloseAll()
{
  for (auto& plt : plots)
    plots.erase(plt.first);
}

Plot::Plot1D& Plot::GetPlot1D(const std::string& name)
{
  if (mPlots1D.count(name) == 0)
    mPlots1D.emplace(name, Plot1D(name));

  return mPlots1D.at(name);
}

Plot::Plot2D& Plot::GetPlot2D(const std::string& name)
{
  if (mPlots2D.count(name) == 0)
    mPlots2D.emplace(name, Plot2D(name));

  return mPlots2D.at(name);
}

Plot::Plot1D::Plot1D(const std::string& name) : mName(name)
{
}

void Plot::Plot1D::Plot(const std::vector<double>& x, const std::vector<double>& y, bool newplot)
{
  PlotCore(x, {y}, {}, newplot);
}

void Plot::Plot1D::Plot(const std::vector<double>& x, const std::vector<std::vector<double>>& ys, bool newplot)
{
  PlotCore(x, ys, {}, newplot);
}

void Plot::Plot1D::Plot(const std::vector<double>& x, const std::vector<double>& y1, const std::vector<double>& y2, bool newplot)
{
  PlotCore(x, {y1}, {y2}, newplot);
}

void Plot::Plot1D::Plot(const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s, bool newplot)
{
  PlotCore(x, y1s, y2s, newplot);
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

void Plot::Plot1D::PlotCore(const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s, bool newplot)
{
  if (newplot)
    mCounter++;

  int y1cnt = y1s.size();
  int y2cnt = y2s.size();
  int ycnt = y1cnt + y2cnt;

  if (!mInitialized || newplot)
    Initialize(ycnt, y1cnt, y2cnt);

  auto& windowPlot = plots[GetName()];
  auto& plot = windowPlot->ui.widget;

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

  auto& windowPlot = plots[GetName()];
  auto& plot = windowPlot->ui.widget;

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
  auto name = GetName();
  auto idx = plots.find(name);
  if (idx != plots.end())
  {
    Reset();
    mInitialized = true;
    return;
  }

  LOG_DEBUG("Initializing plot {}", name);
  plots[name] = std::make_unique<WindowPlot>(name, 1.3, OnClose);
  auto& windowPlot = plots[name];
  windowPlot->move(GetNewPlotPosition(windowPlot.get(), name));
  auto& plot = windowPlot->ui.widget;

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
    plot->yAxis->setNumberPrecision(1);
    // plot->yAxis->setNumberFormat("eb");
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    plot->yAxis->setTicker(logTicker);
  }

  if (mY2Log)
  {
    plot->yAxis2->setScaleType(QCPAxis::stLogarithmic);
    plot->yAxis2->setNumberPrecision(1);
    // plot->yAxis2->setNumberFormat("eb");
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    plot->yAxis2->setTicker(logTicker);
  }

  for (int i = 0; i < ycnt; i++)
  {
    auto graph = plot->graph(i);

    if (i < y1cnt)
    {
      plot->addGraph(plot->xAxis, plot->yAxis);
      graph = plot->graph(i);

      if (mY1names.size() > i)
        graph->setName(QString::fromStdString(mY1names[i]));
      else
        graph->setName(QString::fromStdString("y1_" + to_string(i + 1)));
    }
    else
    {
      plot->addGraph(plot->xAxis, plot->yAxis2);
      graph = plot->graph(i);

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
      graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, pt));
    }
  }

  if (y2cnt > 0)
    plot->yAxis2->setVisible(true);

  windowPlot->show();
  QCoreApplication::processEvents();
  mInitialized = true;
}

std::string Plot::Plot1D::GetName()
{
  return mCounter > 0 ? fmt::format("{}:{}", mName, mCounter) : mName;
}

void Plot::Plot1D::Reset()
{
  auto name = GetName();
  auto idx = plots.find(name);
  if (idx == plots.end())
    return;

  LOG_DEBUG("Resetting plot {}", name);
  auto& windowPlot = idx->second;
  auto& plot = windowPlot->ui.widget;

  for (int i = 0; i < plot->graphCount(); i++)
    plot->graph(i)->data().data()->clear();

  plot->rescaleAxes();
  plot->replot();
  windowPlot->show();
}

Plot::Plot2D::Plot2D(const std::string& name) : mName(name)
{
}

void Plot::Plot2D::Plot(const Mat& z, bool newplot)
{
  std::vector<std::vector<double>> zv = zerovect2(z.rows, z.cols, 0.);
  for (int r = 0; r < z.rows; r++)
    for (int c = 0; c < z.cols; c++)
      zv[r][c] = z.at<float>(r, c);

  PlotCore(zv, newplot);
}

void Plot::Plot2D::Plot(const std::vector<std::vector<double>>& z, bool newplot)
{
  PlotCore(z, newplot);
}

void Plot::Plot2D::PlotCore(const std::vector<std::vector<double>>& z, bool newplot)
{
  if (newplot)
    mCounter++;

  if (!mInitialized || newplot)
    Initialize(z[0].size(), z.size());

  auto& windowPlot = plots[GetName()];
  windowPlot->colorMap->data()->setSize(z[0].size(), z.size());
  windowPlot->colorMap->data()->setRange(QCPRange(mXmin, mXmax), QCPRange(mYmin, mYmax));
  for (int xIndex = 0; xIndex < z[0].size(); ++xIndex)
    for (int yIndex = 0; yIndex < z.size(); ++yIndex)
      windowPlot->colorMap->data()->setCell(xIndex, yIndex, z[z.size() - 1 - yIndex][xIndex]);

  windowPlot->ui.widget->rescaleAxes();
  windowPlot->colorMap->rescaleDataRange(true);
  windowPlot->colorScale->rescaleDataRange(false);
  windowPlot->ui.widget->replot();
  windowPlot->show();

  if (mSavepath.length() > 0)
    windowPlot->ui.widget->savePng(QString::fromStdString(mSavepath), 0, 0, 3, -1);
}

void Plot::Plot2D::Initialize(int xcnt, int ycnt)
{
  auto name = GetName();
  auto idx = plots.find(name);
  if (idx != plots.end())
  {
    Reset();
    mInitialized = true;
    return;
  }

  double colRowRatio = mColRowRatio == 0 ? (double)xcnt / ycnt : mColRowRatio;
  plots[name] = std::make_unique<WindowPlot>(name, colRowRatio, OnClose);
  auto& windowPlot = plots[name];
  windowPlot->move(Plot::GetNewPlotPosition(windowPlot.get(), name));

  auto& plot = windowPlot->ui.widget;
  auto& colorMap = windowPlot->colorMap;
  auto& colorScale = windowPlot->colorScale;

  plot->addGraph();
  plot->axisRect()->setupFullAxesBox(true);
  colorMap = new QCPColorMap(plot->xAxis, plot->yAxis);
  plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
  colorScale = new QCPColorScale(plot);
  plot->plotLayout()->addElement(0, 1, colorScale);
  colorScale->setType(QCPAxis::atRight);
  colorMap->setColorScale(colorScale);
  if (mShowAxisLabels)
  {
    plot->xAxis->setLabel(QString::fromStdString(mXlabel));
    plot->yAxis->setLabel(QString::fromStdString(mYlabel));
    colorScale->axis()->setLabel(QString::fromStdString(mZlabel));
  }
  colorMap->setGradient(mColormapType);
  windowPlot->marginGroup = new QCPMarginGroup(plot);
  plot->axisRect()->setMarginGroup(QCP::msBottom | QCP::msTop, windowPlot->marginGroup);
  colorScale->setMarginGroup(QCP::msBottom | QCP::msTop, windowPlot->marginGroup);
  colorMap->setInterpolate(true);
  plot->xAxis->setTickLabelFont(Plot::fontTicks);
  plot->yAxis->setTickLabelFont(Plot::fontTicks);
  colorScale->axis()->setTickLabelFont(Plot::fontTicks);
  plot->xAxis->setLabelFont(Plot::fontLabels);
  plot->yAxis->setLabelFont(Plot::fontLabels);
  plot->yAxis->setRangeReversed(false);
  colorScale->axis()->setLabelFont(Plot::fontLabels);

  windowPlot->show();
  QCoreApplication::processEvents();
  mInitialized = true;
}

std::string Plot::Plot2D::GetName()
{
  return fmt::format("{}:{}", mName, mCounter);
}

void Plot::Plot2D::Reset()
{
  auto name = GetName();
  auto idx = plots.find(name);
  if (idx == plots.end())
    return;

  LOG_DEBUG("Resetting plot {}", name);
  auto& windowPlot = idx->second;
  auto& plot = windowPlot->ui.widget;
  plot->graph(0)->data().clear();
}
