#pragma once
#include "stdafx.h"
#include "qcustomplot.h"
#include "plotter.h"
#include "WindowPlot.h"

static const QFont fontTicks("Newyork", 9);
static const QFont fontLabels("Newyork", 12);
static const std::vector<QPen> plotPens{ QPen(Qt::gray, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin), QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin), QPen(Qt::blue, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin), QPen(Qt::red, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin) };
static const QPen plotPen(Qt::blue, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
static const QPen plotPen2(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

struct Plot1D : IPlot1D
{
	QCustomPlot* widget;

	inline Plot1D(QCustomPlot* widget) : widget(widget)
	{
		widget->clearPlottables();
		widget->addGraph();
		widget->xAxis->setTickLabelFont(fontTicks);
		widget->yAxis->setTickLabelFont(fontTicks);
		widget->xAxis->setLabelFont(fontLabels);
		widget->yAxis->setLabelFont(fontLabels);
		widget->graph(0)->setPen(plotPen);
		widget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);//allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking
	};

	inline void setupSecondGraph(QString ylabel1, QString ylabel2)
	{
		widget->addGraph(widget->xAxis, widget->yAxis2);
		widget->yAxis2->setVisible(true);
		widget->yAxis2->setLabel(ylabel2);
		widget->yAxis2->setTickLabelFont(fontTicks);
		widget->yAxis2->setLabelFont(fontLabels);
		widget->graph(1)->setPen(plotPen2);
		widget->legend->setVisible(true);
		widget->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom | Qt::AlignRight);
		widget->graph(0)->setName(ylabel1);
		widget->graph(1)->setName(ylabel2);
		//widget->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
		//widget->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
	}

	inline void setupMultipleGraph(std::vector<std::string> ylabels)
	{
		for (int i = 0; i < ylabels.size(); i++)
		{
			if (i) widget->addGraph();
			widget->graph(i)->setPen(plotPens[i]);
			widget->graph(i)->setName(QString::fromStdString(ylabels[i]));
			//widget->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
		}
		widget->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom | Qt::AlignRight);
		widget->legend->setVisible(true);
	}

	inline void setAxisNames(std::string xlabel, std::string ylabel) override
	{
		widget->xAxis->setLabel(QString::fromStdString(xlabel));
		widget->yAxis->setLabel(QString::fromStdString(ylabel));
	}

	inline void setAxisNames(std::string xlabel, std::string ylabel1, std::string ylabel2) override
	{
		widget->xAxis->setLabel(QString::fromStdString(xlabel));
		widget->yAxis->setLabel(QString::fromStdString(ylabel1));
		widget->yAxis2->setLabel(QString::fromStdString(ylabel2));
		if (widget->graphCount() < 2) setupSecondGraph(QString::fromStdString(ylabel1), QString::fromStdString(ylabel2));
	}

	inline void setAxisNames(std::string xlabel, std::string ylabel, std::vector<std::string> ylabels) override
	{
		widget->xAxis->setLabel(QString::fromStdString(xlabel));
		widget->yAxis->setLabel(QString::fromStdString(ylabel));
		if (widget->graphCount() < ylabels.size()) setupMultipleGraph(ylabels);
	}

	inline void plot(const std::vector<double>& x, const std::vector<double>& y) override
	{
		widget->graph(0)->setData(QVector<double>::fromStdVector(x), QVector<double>::fromStdVector(y));
		widget->rescaleAxes();
		widget->replot();
	}

	inline void plot(const std::vector<double>& x, const std::vector<double>& y1, const std::vector<double>& y2) override
	{
		widget->graph(0)->setData(QVector<double>::fromStdVector(x), QVector<double>::fromStdVector(y1));
		widget->graph(1)->setData(QVector<double>::fromStdVector(x), QVector<double>::fromStdVector(y2));
		widget->rescaleAxes();
		widget->replot();
	}

	inline void plot(const std::vector<double>& x, const std::vector<std::vector<double>>& ys) override
	{
		for (int i = 0; i < ys.size(); i++)
		{
			widget->graph(i)->setData(QVector<double>::fromStdVector(x), QVector<double>::fromStdVector(ys[i]));
		}
		widget->rescaleAxes();
		widget->replot();
	}

	inline void plot(const double x, const double y) override
	{
		widget->graph(0)->addData(x, y);
		widget->rescaleAxes();
		widget->replot();
	}

	inline void plot(const double x, const double y1, const double y2) override
	{
		widget->graph(0)->addData(x, y1);
		widget->graph(1)->addData(x, y2);
		widget->rescaleAxes();
		widget->replot();
	}

	inline void clear(bool second) override
	{
		widget->graph(0)->data()->clear();
		if (second) widget->graph(1)->data()->clear();
	}

	inline void save(std::string path) override
	{
		widget->savePng(QString::fromStdString(path), 0, 0, 3, -1);
	}
};

struct Plotter2D : IPlot2D
{
	std::vector<WindowPlot*> windowPlots;

	inline Plotter2D()
	{
		
	};

	inline ~Plotter2D()
	{
		//delete windowPlots[index];//destroy WindowPlot
		//windowPlots->erase(windowPlots->begin() + index);//remove from vector of WindowPlot*
	}

	inline void plot(const std::vector<std::vector<double>>& z, std::string xlabel, std::string ylabel, std::string zlabel, double xmin, double xmax, double ymin, double ymax) override
	{
		QString qxlabel = QString::fromStdString(xlabel);
		QString qylabel = QString::fromStdString(ylabel);
		QString qzlabel = QString::fromStdString(zlabel);
		windowPlots.push_back(new WindowPlot());
		int index = windowPlots.size() - 1;
		windowPlots[index]->show();
		windowPlots[index]->ui.widget->addGraph();//create graph
		windowPlots[index]->ui.widget->axisRect()->setupFullAxesBox(true);//configure axis rect
		windowPlots[index]->ui.widget->xAxis->setLabel(qxlabel);//give the axes some labels
		windowPlots[index]->ui.widget->yAxis->setLabel(qylabel);//give the axes some labels
		QCPColorMap* colorMap = new QCPColorMap(windowPlots[index]->ui.widget->xAxis, windowPlots[index]->ui.widget->yAxis);//set up the QCPColorMap
		colorMap->data()->setSize(z[0].size(), z.size());//we want the color map to have nx * ny data points
		colorMap->data()->setRange(QCPRange(xmin, xmax), QCPRange(ymin, ymax));//and span the coordinate range in both key (x) and value (y) dimensions
		windowPlots[index]->ui.widget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);//allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking
		QCPColorScale*colorScale = new QCPColorScale(windowPlots[index]->ui.widget);//add a color scale
		windowPlots[index]->ui.widget->plotLayout()->addElement(0, 1, colorScale);//add it to the right of the main axis rect
		colorScale->setType(QCPAxis::atRight);//scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
		colorMap->setColorScale(colorScale);//associate the color map with the color scale
		colorScale->axis()->setLabel(qzlabel);//add the z value name
		colorMap->setGradient(QCPColorGradient::gpJet);//set the color gradient of the color map to one of the presets
		QCPMarginGroup* marginGroup = new QCPMarginGroup(windowPlots[index]->ui.widget);//make sure the axis rect and color scale synchronize their bottom and top margins (so they line up)
		windowPlots[index]->ui.widget->axisRect()->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);//align plot
		colorScale->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);//align colorbar
		colorMap->setInterpolate(true);//interpolate bro
		windowPlots[index]->ui.widget->xAxis->setTickLabelFont(fontTicks);
		windowPlots[index]->ui.widget->yAxis->setTickLabelFont(fontTicks);
		colorScale->axis()->setTickLabelFont(fontTicks);
		windowPlots[index]->ui.widget->xAxis->setLabelFont(fontLabels);
		windowPlots[index]->ui.widget->yAxis->setLabelFont(fontLabels);
		windowPlots[index]->ui.widget->yAxis->setRangeReversed(true);
		colorScale->axis()->setLabelFont(fontLabels);

		for (int xIndex = 0; xIndex < z[0].size(); ++xIndex)
		{
			for (int yIndex = 0; yIndex < z.size(); ++yIndex)
			{
				colorMap->data()->setCell(xIndex, yIndex, z[yIndex][xIndex]);
			}
		}
		colorMap->rescaleDataRange();//rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
		windowPlots[index]->ui.widget->rescaleAxes();
		colorScale->rescaleDataRange(0);
		windowPlots[index]->ui.widget->replot();
	}

	inline void save(std::string path, int index) override
	{
		windowPlots[index]->ui.widget->savePng(QString::fromStdString(path), 0, 0, 3, -1);
	}
};