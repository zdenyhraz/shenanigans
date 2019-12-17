#pragma once
#include "stdafx.h"
#include "qcustomplot.h"

struct PlotSettings
{
	QString xlabel;
	QString ylabel;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
};

inline void makePlot(const std::vector<double>& x, const std::vector<double>& y, QCustomPlot* plot, PlotSettings* plotset = nullptr)//1D plot
{
	QVector<double> qx = QVector<double>::fromStdVector(x);
	QVector<double> qy = QVector<double>::fromStdVector(y);

	// create graph and assign data to it:
	plot->addGraph();
	plot->graph(0)->setData(qx, qy);

	// give the axes some labels:
	plot->xAxis->setLabel("x");
	plot->yAxis->setLabel("y");

	// Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
	plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

	// set axes ranges, so we see all data:
	plot->rescaleAxes();

	plot->replot();
}

inline void rePlot(const std::vector<double>& x, const std::vector<double>& y, QCustomPlot* plot)//1D plot
{
	QVector<double> qx = QVector<double>::fromStdVector(x);
	QVector<double> qy = QVector<double>::fromStdVector(y);

	plot->graph(0)->setData(qx, qy);
	plot->rescaleAxes();
	plot->replot();
}


inline void plot(std::vector<double>& x, std::vector<double>& y, std::vector<std::vector<double>>& z, QCustomPlot* plot)//2D plot
{
	
}