#include "Plot1D.hpp"

std::unordered_map<std::string, Plot1D> Plot1D::mPlots;
std::string Plot1D::mCurrentPlot = "Plot1D";

Plot1D::Plot1D(const std::string& name) : mName(name)
{
}

Plot1D& Plot1D::GetPlot(const std::string& name)
{
  mPlots.try_emplace(name, name);
  mCurrentPlot = name;
  return mPlots.at(name);
}

void Plot1D::PlotCore(const std::vector<double>& x, const std::vector<std::vector<double>>& y1s, const std::vector<std::vector<double>>& y2s)
{
  PROFILE_FUNCTION;
  int y1cnt = y1s.size();
  int y2cnt = y2s.size();
  int ycnt = y1cnt + y2cnt;

  Initialize(ycnt, y1cnt, y2cnt, true);
  auto& windowPlot = Plot::plots[mName];
  auto& plot = windowPlot->ui.widget;

  for (int i = 0; i < ycnt; i++)
  {
    if (i < y1cnt)
      plot->graph(i)->setData(QVector<double>::fromStdVector(x), QVector<double>::fromStdVector(y1s[i]));
    else
      plot->graph(i)->setData(QVector<double>::fromStdVector(x), QVector<double>::fromStdVector(y2s[i - y1cnt]));
  }

  plot->rescaleAxes();

  if (mYmin != std::numeric_limits<double>::lowest())
    plot->yAxis->setRangeLower(mYmin);
  if (mYmax != std::numeric_limits<double>::max())
    plot->yAxis->setRangeUpper(mYmax);
  if (mY2min != std::numeric_limits<double>::lowest())
    plot->yAxis2->setRangeLower(mY2min);
  if (mY2max != std::numeric_limits<double>::max())
    plot->yAxis2->setRangeUpper(mY2max);

  plot->replot();
  windowPlot->show();
  QCoreApplication::processEvents();

  if (not mSavepath.empty())
  {
    LOG_DEBUG("Saving plot to {} ...", std::filesystem::weakly_canonical(mSavepath).string());
    plot->savePng(QString::fromStdString(mSavepath), 0, 0, 3, -1);
  }
}

void Plot1D::PlotCore(double x, const std::vector<double>& y1s, const std::vector<double>& y2s)
{
  PROFILE_FUNCTION;
  int y1cnt = y1s.size();
  int y2cnt = y2s.size();
  int ycnt = y1cnt + y2cnt;

  Initialize(ycnt, y1cnt, y2cnt, false);
  auto& windowPlot = Plot::plots[mName];
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

  if (mYmin != std::numeric_limits<double>::lowest())
    plot->yAxis->setRangeLower(mYmin);
  if (mYmax != std::numeric_limits<double>::max())
    plot->yAxis->setRangeUpper(mYmax);
  if (mY2min != std::numeric_limits<double>::lowest())
    plot->yAxis2->setRangeLower(mY2min);
  if (mY2max != std::numeric_limits<double>::max())
    plot->yAxis2->setRangeUpper(mY2max);

  plot->replot();
  windowPlot->show();
  QCoreApplication::processEvents();

  if (not mSavepath.empty())
  {
    LOG_DEBUG("Saving plot to {} ...", std::filesystem::weakly_canonical(mSavepath).string());
    plot->savePng(QString::fromStdString(mSavepath), 0, 0, 3, -1);
  }
}

void Plot1D::Initialize(int ycnt, int y1cnt, int y2cnt, bool clear)
{
  PROFILE_FUNCTION;
  auto idx = Plot::plots.find(mName);
  if (idx != Plot::plots.end())
  {
    if (clear)
      ClearCore();
    return;
  }

  LOG_TRACE("Initializing plot {}", mName);
  Plot::plots[mName] = std::make_unique<WindowPlot>(mName, 1.3, Plot::OnClose);
  auto& windowPlot = Plot::plots[mName];
  windowPlot->move(Plot::GetNewPlotPosition(windowPlot.get(), mName));
  auto& plot = windowPlot->ui.widget;

  plot->setNoAntialiasingOnDrag(true);
  plot->xAxis->setTickLabelFont(Plot::fontTicks);
  plot->yAxis->setTickLabelFont(Plot::fontTicks);
  plot->yAxis2->setTickLabelFont(Plot::fontTicks);
  plot->xAxis->setLabelFont(Plot::fontLabels);
  plot->yAxis->setLabelFont(Plot::fontLabels);
  plot->yAxis2->setLabelFont(Plot::fontLabels);
  plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
  plot->xAxis->setLabel(QString::fromStdString(mXlabel));
  plot->yAxis->setLabel(QString::fromStdString(mYlabel));
  plot->yAxis2->setLabel(QString::fromStdString(mY2label));
  plot->legend->setVisible(mLegendVisible && ycnt > 1);
  plot->legend->setFont(Plot::fontLegend);

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

  if (mYLogarithmic)
  {
    plot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    plot->yAxis->setTicker(logTicker);
  }

  if (mY2Logarithmic)
  {
    plot->yAxis2->setScaleType(QCPAxis::stLogarithmic);
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    plot->yAxis2->setTicker(logTicker);
  }

  for (int i = 0; i < ycnt; i++)
  {
    QCPGraph* graph = nullptr;

    if (i < y1cnt)
    {
      graph = plot->addGraph(plot->xAxis, plot->yAxis);

      if (mYnames.size() > static_cast<size_t>(i))
        graph->setName(QString::fromStdString(mYnames[i]));
      else
        graph->setName(QString::fromStdString("y1_" + std::to_string(i + 1)));
    }
    else
    {
      graph = plot->addGraph(plot->xAxis, plot->yAxis2);

      if (mY2names.size() > static_cast<size_t>(i - y1cnt))
        graph->setName(QString::fromStdString(mY2names[i - y1cnt]));
      else
        graph->setName(QString::fromStdString("y2_" + std::to_string(i - y1cnt + 1)));
    }

    if (static_cast<size_t>(i) < mPens.size())
      graph->setPen(mPens[i]);
    else
      graph->setPen(QPen(QColor(Random::Rand() % 255, Random::Rand() % 255, Random::Rand() % 255), Plot::pt, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    if (mScatterStyle)
    {
      graph->setLineStyle(QCPGraph::lsNone);
      graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, Plot::pt));
    }
  }

  if (y2cnt > 0)
    plot->yAxis2->setVisible(true);

  windowPlot->show();
}

void Plot1D::ClearCore()
{
  auto idx = Plot::plots.find(mName);
  if (idx != Plot::plots.end())
  {
    LOG_TRACE("Clearing plot {}", mName);
    auto& windowPlot = idx->second;
    auto& plot = windowPlot->ui.widget;
    for (int i = 0; i < plot->graphCount(); i++)
      plot->graph(i)->data().data()->clear();
  }
}
