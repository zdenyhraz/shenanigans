#pragma once
#include "stdafx.h"
#include "qcustomplot.h"
#include "plotter.h"

static const QFont fontTicks("Newyork", 9);
static const QFont fontLabels("Newyork", 12);
static const QPen plotPen(Qt::blue, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
static const QPen plotPen2(Qt::red, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

struct Plot1D : AbstractPlot1D
{
	QCustomPlot* widget;

	inline Plot1D(QCustomPlot* widget, QString xlabel = "x", QString ylabel = "y", QString ylabel2 = "none") : widget(widget)
	{
		widget->clearPlottables();

		widget->addGraph();
		widget->xAxis->setLabel(xlabel);
		widget->yAxis->setLabel(ylabel);
		widget->xAxis->setTickLabelFont(fontTicks);
		widget->yAxis->setTickLabelFont(fontTicks);
		widget->xAxis->setLabelFont(fontLabels);
		widget->yAxis->setLabelFont(fontLabels);
		widget->graph(0)->setPen(plotPen);
		if (ylabel2 != "none") setupSecondGraph(ylabel, ylabel2);
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
		widget->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
		widget->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
	}

	inline void setAxisNames(std::string xlabel, std::string ylabel) override
	{
		QString qxlabel = QString::fromStdString(xlabel);
		QString qylabel = QString::fromStdString(ylabel);
		widget->xAxis->setLabel(qxlabel);
		widget->yAxis->setLabel(qylabel);
	}

	inline void setAxisNames(std::string xlabel, std::string ylabel1, std::string ylabel2) override
	{
		QString qxlabel = QString::fromStdString(xlabel);
		QString qylabel1 = QString::fromStdString(ylabel1);
		QString qylabel2 = QString::fromStdString(ylabel2);
		widget->xAxis->setLabel(qxlabel);
		widget->yAxis->setLabel(qylabel1);
		widget->yAxis2->setLabel(qylabel2);
		setupSecondGraph(qylabel1, qylabel2);
	}

	inline void plot(const std::vector<double>& x, const std::vector<double>& y) override
	{
		QVector<double> qx = QVector<double>::fromStdVector(x);
		QVector<double> qy = QVector<double>::fromStdVector(y);
		widget->graph(0)->setData(qx, qy);
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

	inline void plot(const std::vector<double>& x, const std::vector<double>& y1, const std::vector<double>& y2) override
	{
		QVector<double> qx = QVector<double>::fromStdVector(x);
		QVector<double> qy1 = QVector<double>::fromStdVector(y1);
		QVector<double> qy2 = QVector<double>::fromStdVector(y2);
		widget->graph(0)->setData(qx, qy1);
		widget->graph(1)->setData(qx, qy2);
		widget->rescaleAxes();
		widget->replot();
	}

	inline void save(std::string path) override
	{
		widget->savePng(QString::fromStdString(path), 0, 0, 3, -1);
	}
};

struct Plot2D : AbstractPlot2D
{
	QCustomPlot* widget;
	QCPColorMap* colorMap;
	QCPColorScale* colorScale;
	QCPMarginGroup* marginGroup;
	int nx;
	int ny;

	inline Plot2D(QCustomPlot* widget, QString xlabel, QString ylabel, QString zlabel) : widget(widget)
	{
		widget->clearPlottables();
		widget->addGraph();//create graph
		widget->axisRect()->setupFullAxesBox(true);//configure axis rect
		widget->xAxis->setLabel(xlabel);//give the axes some labels
		widget->yAxis->setLabel(ylabel);//give the axes some labels
		colorMap = new QCPColorMap(widget->xAxis, widget->yAxis);//set up the QCPColorMap
		colorMap->data()->setSize(100, 100);//we want the color map to have nx * ny data points
		colorMap->data()->setRange(QCPRange(0, 1), QCPRange(0, 1));//and span the coordinate range in both key (x) and value (y) dimensions
		widget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);//allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking
		colorScale = new QCPColorScale(widget);//add a color scale
		widget->plotLayout()->addElement(0, 1, colorScale);//add it to the right of the main axis rect
		colorScale->setType(QCPAxis::atRight);//scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
		colorMap->setColorScale(colorScale);//associate the color map with the color scale
		colorScale->axis()->setLabel(zlabel);//add the z value name
		colorMap->setGradient(QCPColorGradient::gpJet);//set the color gradient of the color map to one of the presets
		marginGroup = new QCPMarginGroup(widget);//make sure the axis rect and color scale synchronize their bottom and top margins (so they line up)
		widget->axisRect()->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);//align plot
		colorScale->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);//align colorbar
		colorMap->setInterpolate(true);//interpolate bro
		widget->xAxis->setTickLabelFont(fontTicks);
		widget->yAxis->setTickLabelFont(fontTicks);
		colorScale->axis()->setTickLabelFont(fontTicks);
		widget->xAxis->setLabelFont(fontLabels);
		widget->yAxis->setLabelFont(fontLabels);
		//widget->yAxis->setRangeReversed(true);
		colorScale->axis()->setLabelFont(fontLabels);
	};

	inline void setAxisNames(std::string xlabel, std::string ylabel, std::string zlabel) override
	{
		QString qxlabel = QString::fromStdString(xlabel);
		QString qylabel = QString::fromStdString(ylabel);
		QString qzlabel = QString::fromStdString(zlabel);
		widget->xAxis->setLabel(qxlabel);
		widget->yAxis->setLabel(qylabel);
		colorScale->axis()->setLabel(qzlabel);
	}

	inline void setSize(int nxx, int nyy) override
	{
		nx = nxx;
		ny = nyy;
		colorMap->data()->setSize(nx, ny);
	}

	inline void setRange(double xmin, double xmax, double ymin, double ymax) override
	{
		colorMap->data()->setRange(QCPRange(xmin, xmax), QCPRange(ymin, ymax));
	}

	inline void plot(const std::vector<std::vector<double>>& z) override
	{
		setSize(z[0].size(), z.size());
		for (int xIndex = 0; xIndex < nx; ++xIndex)
		{
			for (int yIndex = 0; yIndex < ny; ++yIndex)
			{
				colorMap->data()->setCell(xIndex, yIndex, z[yIndex][xIndex]);
			}
		}
		colorMap->rescaleDataRange();//rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
		widget->rescaleAxes();
		colorScale->rescaleDataRange(0);
		widget->replot();
	}

	inline void save(std::string path) override
	{
		widget->savePng(QString::fromStdString(path), 0, 0, 3, -1);
	}
};