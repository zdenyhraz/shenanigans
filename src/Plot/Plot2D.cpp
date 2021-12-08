#include "Log/Logger.h"
#include "Core/functionsBaseSTL.h"
#include "Plot2D.h"

std::map<std::string, Plot2D> Plot2D::mPlots;
std::string Plot2D::mCurrentPlot = "Plot2D";

Plot2D::Plot2D(const std::string& name) : mName(name)
{
}

Plot2D& Plot2D::GetPlot(const std::string& mName)
{
  if (mPlots.count(mName) == 0)
    mPlots.emplace(mName, Plot2D(mName));

  mCurrentPlot = mName;
  return mPlots.at(mName);
}

void Plot2D::PlotCore(const cv::Mat& z)
{
  std::vector<std::vector<double>> zv = zerovect2(z.rows, z.cols, 0.);

  if (z.depth() != CV_32F)
  {
    cv::Mat zf;
    z.convertTo(zf, CV_32F);
    for (int r = 0; r < z.rows; r++)
      for (int c = 0; c < z.cols; c++)
        zv[r][c] = zf.at<float>(r, c);
  }
  else
  {
    for (int r = 0; r < z.rows; r++)
      for (int c = 0; c < z.cols; c++)
        zv[r][c] = z.at<float>(r, c);
  }

  PlotCore(zv);
}

void Plot2D::PlotCore(const std::vector<std::vector<double>>& z)
{
  Initialize(z[0].size(), z.size());
  auto& windowPlot = Plot::plots[mName];
  windowPlot->colorMap->data()->setSize(z[0].size(), z.size());
  windowPlot->colorMap->data()->setRange(QCPRange(mXmin, mXmax), QCPRange(mYmin, mYmax));
  // the cell with indices (0, 0) is in the bottom left corner and the cell with
  // indices(keySize - 1, valueSize - 1) is in the top right corner of the colormap
  for (size_t xIndex = 0; xIndex < z[0].size(); ++xIndex)
    for (size_t yIndex = 0; yIndex < z.size(); ++yIndex)
      windowPlot->colorMap->data()->setCell(xIndex, yIndex, z[z.size() - 1 - yIndex][xIndex]);

  windowPlot->ui.widget->rescaleAxes();
  windowPlot->colorMap->rescaleDataRange(true);
  windowPlot->colorScale->rescaleDataRange(false);
  windowPlot->ui.widget->replot();
  windowPlot->show();

  if (mSavepath.length() > 0)
    windowPlot->ui.widget->savePng(QString::fromStdString(mSavepath), 0, 0, 3, -1);
}

void Plot2D::Initialize(int xcnt, int ycnt)
{
  auto idx = Plot::plots.find(mName);
  if (idx != Plot::plots.end())
  {
    ClearCore();
    return;
  }

  double colRowRatio = mColRowRatio == 0 ? (double)xcnt / ycnt : mColRowRatio;
  if (mShowAxisLabels)
    colRowRatio *= 1.1;
  Plot::plots[mName] = std::make_unique<WindowPlot>(mName, colRowRatio, Plot::OnClose);
  auto& windowPlot = Plot::plots[mName];
  windowPlot->move(Plot::GetNewPlotPosition(windowPlot.get(), mName));

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
}

void Plot2D::ClearCore()
{
  auto idx = Plot::plots.find(mName);
  if (idx != Plot::plots.end())
  {
    LOG_TRACE("Clearing plot {}", mName);
    auto& windowPlot = idx->second;
    auto& plot = windowPlot->ui.widget;
    plot->graph(0)->data().clear();
  }
}
