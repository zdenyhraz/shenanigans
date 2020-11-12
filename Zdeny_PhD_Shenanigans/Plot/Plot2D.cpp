#include "stdafx.h"
#include "Plot2D.h"
#include "Plot1D.h"

std::function<void(std::string)> Plot2D::OnClose = [](std::string name) {
  auto idx = Plot::plots.find(name);
  if (idx != Plot::plots.end())
  {
    delete idx->second;
    Plot::plots.erase(idx);
  }
};

void Plot2D::CloseAll()
{
  for (auto& plt : Plot::plots)
  {
    delete plt.second;
    Plot::plots.erase(plt.first);
  }
}

void Plot2D::plotcore(const std::vector<std::vector<double>>& z, std::string name, std::string xlabel, std::string ylabel, std::string zlabel,
                      double xmin, double xmax, double ymin, double ymax, double colRowRatio, std::string savepath)
{
  WindowPlot* windowPlot;
  auto idx = Plot::plots.find(name);
  if (idx != Plot::plots.end())
  {
    windowPlot = idx->second;
    windowPlot->ui.widget->graph(0)->data().clear();
  }
  else
  {
    if (colRowRatio == 0)
      colRowRatio = (double)z[0].size() / z.size();

    windowPlot = new WindowPlot(name, colRowRatio, OnClose);
    windowPlot->move(Plot::GetNewPlotPosition(windowPlot));
    Plot::plots[name] = windowPlot;
    SetupGraph(windowPlot, xlabel, ylabel, zlabel);
  }

  windowPlot->colorMap->data()->setSize(z[0].size(), z.size());
  windowPlot->colorMap->data()->setRange(QCPRange(xmin, xmax), QCPRange(ymin, ymax));
  for (int xIndex = 0; xIndex < z[0].size(); ++xIndex)
    for (int yIndex = 0; yIndex < z.size(); ++yIndex)
      windowPlot->colorMap->data()->setCell(xIndex, yIndex, z[z.size() - 1 - yIndex][xIndex]);

  windowPlot->ui.widget->rescaleAxes();
  windowPlot->colorMap->rescaleDataRange(true);
  windowPlot->colorScale->rescaleDataRange(false);
  windowPlot->ui.widget->replot();
  windowPlot->show();

  if (savepath != "")
    windowPlot->ui.widget->savePng(QString::fromStdString(savepath), 0, 0, 3, -1);
}

void Plot2D::SetupGraph(WindowPlot* windowPlot, std::string xlabel, std::string ylabel, std::string zlabel)
{
  // create graph
  windowPlot->ui.widget->addGraph();
  // configure axis rect
  windowPlot->ui.widget->axisRect()->setupFullAxesBox(true);
  // give the axes some labels
  // windowPlot->ui.widget->xAxis->setLabel(QString::fromStdString(xlabel));
  // give the axes some labels
  // windowPlot->ui.widget->yAxis->setLabel(QString::fromStdString(ylabel));
  // set up the QCPColorMap
  windowPlot->colorMap = new QCPColorMap(windowPlot->ui.widget->xAxis, windowPlot->ui.widget->yAxis);
  // allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking
  windowPlot->ui.widget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
  // add a color scale
  windowPlot->colorScale = new QCPColorScale(windowPlot->ui.widget);
  // add it to the right of the main axis rect
  windowPlot->ui.widget->plotLayout()->addElement(0, 1, windowPlot->colorScale);
  // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
  windowPlot->colorScale->setType(QCPAxis::atRight);
  // associate the color map with the color scale
  windowPlot->colorMap->setColorScale(windowPlot->colorScale);
  // add the z value name
  // windowPlot->colorScale->axis()->setLabel( QString::fromStdString( zlabel ) );
  // set the color gradient of the color map to one of the presets
  windowPlot->colorMap->setGradient(QCPColorGradient::gpJet);
  // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up)
  windowPlot->marginGroup = new QCPMarginGroup(windowPlot->ui.widget);
  // align plot
  windowPlot->ui.widget->axisRect()->setMarginGroup(QCP::msBottom | QCP::msTop, windowPlot->marginGroup);
  // align colorbar
  windowPlot->colorScale->setMarginGroup(QCP::msBottom | QCP::msTop, windowPlot->marginGroup);
  // interpolate bro
  windowPlot->colorMap->setInterpolate(true);
  windowPlot->ui.widget->xAxis->setTickLabelFont(Plot::fontTicks);
  windowPlot->ui.widget->yAxis->setTickLabelFont(Plot::fontTicks);
  windowPlot->colorScale->axis()->setTickLabelFont(Plot::fontTicks);
  windowPlot->ui.widget->xAxis->setLabelFont(Plot::fontLabels);
  windowPlot->ui.widget->yAxis->setLabelFont(Plot::fontLabels);
  windowPlot->ui.widget->yAxis->setRangeReversed(false);
  windowPlot->colorScale->axis()->setLabelFont(Plot::fontLabels);
  windowPlot->show();
  QCoreApplication::processEvents();
}
