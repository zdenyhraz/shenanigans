#pragma once
#include "stdafx.h"
#include "qcustomplot.h"
#include "plotter.h"
#include "Gui/WindowPlot.h"
#include "plotterqt.h"

namespace Plot2D {

static std::unordered_map<int, std::shared_ptr<WindowPlot>> windowPlots;

static std::function<void( WindowPlot * )> OnClose = []( WindowPlot *winplt )
{
	//winplt->colorMap->deleteLater();
	//winplt->colorScale->deleteLater();
	//winplt->marginGroup->deleteLater();
	//delete winplt;
};

inline static void plot( const std::vector<std::vector<double>> &z, std::string xlabel = "x", std::string ylabel = "y", std::string zlabel = "z", double xmin = 0, double xmax = 1, double ymin = 0, double ymax = 1, std::string savepath = "" )
{
	std::shared_ptr<WindowPlot> windowPlot = std::make_shared<WindowPlot>( OnClose );
	windowPlot->ui.widget->addGraph();//create graph
	windowPlot->ui.widget->axisRect()->setupFullAxesBox( true ); //configure axis rect
	windowPlot->ui.widget->xAxis->setLabel( QString::fromStdString( xlabel ) ); //give the axes some labels
	windowPlot->ui.widget->yAxis->setLabel( QString::fromStdString( ylabel ) ); //give the axes some labels
	windowPlot->colorMap = std::make_shared<QCPColorMap>( windowPlot->ui.widget->xAxis, windowPlot->ui.widget->yAxis ); //set up the QCPColorMap
	windowPlot->colorMap->data()->setSize( z[0].size(), z.size() ); //we want the color map to have nx * ny data points
	windowPlot->colorMap->data()->setRange( QCPRange( xmin, xmax ), QCPRange( ymin, ymax ) ); //and span the coordinate range in both key (x) and value (y) dimensions
	windowPlot->ui.widget->setInteractions( QCP::iRangeDrag | QCP::iRangeZoom ); //allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking
	windowPlot->colorScale = std::make_shared<QCPColorScale>( windowPlot->ui.widget ); //add a color scale
	windowPlot->ui.widget->plotLayout()->addElement( 0, 1, windowPlot->colorScale.get() ); //add it to the right of the main axis rect
	windowPlot->colorScale->setType( QCPAxis::atRight ); //scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
	windowPlot->colorMap->setColorScale( windowPlot->colorScale.get() ); //associate the color map with the color scale
	windowPlot->colorScale->axis()->setLabel( QString::fromStdString( zlabel ) ); //add the z value name
	windowPlot->colorMap->setGradient( QCPColorGradient::gpJet ); //set the color gradient of the color map to one of the presets
	windowPlot->marginGroup = std::make_shared<QCPMarginGroup>( windowPlot->ui.widget ); //make sure the axis rect and color scale synchronize their bottom and top margins (so they line up)
	windowPlot->ui.widget->axisRect()->setMarginGroup( QCP::msBottom | QCP::msTop, windowPlot->marginGroup.get() ); //align plot
	windowPlot->colorScale->setMarginGroup( QCP::msBottom | QCP::msTop, windowPlot->marginGroup.get() ); //align colorbar
	windowPlot->colorMap->setInterpolate( true ); //interpolate bro
	windowPlot->ui.widget->xAxis->setTickLabelFont( fontTicks );
	windowPlot->ui.widget->yAxis->setTickLabelFont( fontTicks );
	windowPlot->colorScale->axis()->setTickLabelFont( fontTicks );
	windowPlot->ui.widget->xAxis->setLabelFont( fontLabels );
	windowPlot->ui.widget->yAxis->setLabelFont( fontLabels );
	windowPlot->ui.widget->yAxis->setRangeReversed( true );
	windowPlot->colorScale->axis()->setLabelFont( fontLabels );
	for ( int xIndex = 0; xIndex < z[0].size(); ++xIndex )
	{
		for ( int yIndex = 0; yIndex < z.size(); ++yIndex )
		{
			windowPlot->colorMap->data()->setCell( xIndex, yIndex, z[yIndex][xIndex] );
		}
	}
	windowPlot->colorMap->rescaleDataRange();//rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
	windowPlot->ui.widget->rescaleAxes();
	windowPlot->colorScale->rescaleDataRange( 0 );
	windowPlot->ui.widget->replot();
	windowPlot->show();

	if ( savepath != "" )
	{
		windowPlot->ui.widget->savePng( QString::fromStdString( savepath ), 0, 0, 3, -1 );
	}

	windowPlots.insert( std::make_pair( ( int )windowPlot.get(), std::move( windowPlot ) ) );
}

inline static void plot( const Mat &z, std::string xlabel = "x", std::string ylabel = "y", std::string zlabel = "z", double xmin = 0, double xmax = 1, double ymin = 0, double ymax = 1, std::string savepath = "" )
{
	return plot( matToVect2( z ), xlabel, ylabel, zlabel, xmin, xmax, ymin, ymax, savepath );
}


};