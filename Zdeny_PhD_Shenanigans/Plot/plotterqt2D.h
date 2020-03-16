#pragma once
#include "stdafx.h"
#include "qcustomplot.h"
#include "plotter.h"
#include "Gui/WindowPlot.h"
#include "plotterqt.h"

// TODO:
// plot with just Z and "name" parameter - similar to showimg() - if "name" already exists, then just update it
// free up resources when window is closed - callback? delete from vec n shit

struct Plot2D : IPlot2D
{
	std::vector<std::shared_ptr<WindowPlot>> windowPlots;

	inline Plot2D()
	{

	};

	inline ~Plot2D()
	{

	}

	inline void plot( const std::vector<std::vector<double>> &z, std::string xlabel = "x", std::string ylabel = "y", std::string zlabel = "z", double xmin = 0, double xmax = 1, double ymin = 0, double ymax = 1 ) override
	{
		windowPlots.push_back( std::make_shared<WindowPlot>() );
		int index = windowPlots.size() - 1;
		windowPlots[index]->show();
		windowPlots[index]->ui.widget->addGraph();//create graph
		windowPlots[index]->ui.widget->axisRect()->setupFullAxesBox( true ); //configure axis rect
		windowPlots[index]->ui.widget->xAxis->setLabel( QString::fromStdString( xlabel ) ); //give the axes some labels
		windowPlots[index]->ui.widget->yAxis->setLabel( QString::fromStdString( ylabel ) ); //give the axes some labels
		std::shared_ptr<QCPColorMap> colorMap = std::make_shared<QCPColorMap>( windowPlots[index]->ui.widget->xAxis, windowPlots[index]->ui.widget->yAxis ); //set up the QCPColorMap
		colorMap->data()->setSize( z[0].size(), z.size() ); //we want the color map to have nx * ny data points
		colorMap->data()->setRange( QCPRange( xmin, xmax ), QCPRange( ymin, ymax ) ); //and span the coordinate range in both key (x) and value (y) dimensions
		windowPlots[index]->ui.widget->setInteractions( QCP::iRangeDrag | QCP::iRangeZoom ); //allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking
		std::shared_ptr<QCPColorScale> colorScale = std::make_shared<QCPColorScale>( windowPlots[index]->ui.widget ); //add a color scale
		windowPlots[index]->ui.widget->plotLayout()->addElement( 0, 1, colorScale.get() ); //add it to the right of the main axis rect
		colorScale->setType( QCPAxis::atRight ); //scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
		colorMap->setColorScale( colorScale.get() ); //associate the color map with the color scale
		colorScale->axis()->setLabel( QString::fromStdString( zlabel ) ); //add the z value name
		colorMap->setGradient( QCPColorGradient::gpJet ); //set the color gradient of the color map to one of the presets
		std::shared_ptr<QCPMarginGroup> marginGroup = std::make_shared<QCPMarginGroup>( windowPlots[index]->ui.widget ); //make sure the axis rect and color scale synchronize their bottom and top margins (so they line up)
		windowPlots[index]->ui.widget->axisRect()->setMarginGroup( QCP::msBottom | QCP::msTop, marginGroup.get() ); //align plot
		colorScale->setMarginGroup( QCP::msBottom | QCP::msTop, marginGroup.get() ); //align colorbar
		colorMap->setInterpolate( true ); //interpolate bro
		windowPlots[index]->ui.widget->xAxis->setTickLabelFont( fontTicks );
		windowPlots[index]->ui.widget->yAxis->setTickLabelFont( fontTicks );
		colorScale->axis()->setTickLabelFont( fontTicks );
		windowPlots[index]->ui.widget->xAxis->setLabelFont( fontLabels );
		windowPlots[index]->ui.widget->yAxis->setLabelFont( fontLabels );
		windowPlots[index]->ui.widget->yAxis->setRangeReversed( true );
		colorScale->axis()->setLabelFont( fontLabels );
		for ( int xIndex = 0; xIndex < z[0].size(); ++xIndex )
		{
			for ( int yIndex = 0; yIndex < z.size(); ++yIndex )
			{
				colorMap->data()->setCell( xIndex, yIndex, z[yIndex][xIndex] );
			}
		}
		colorMap->rescaleDataRange();//rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
		windowPlots[index]->ui.widget->rescaleAxes();
		colorScale->rescaleDataRange( 0 );
		windowPlots[index]->ui.widget->replot();
	}

	inline void clear() override
	{

	}

	inline void save( std::string path, int index ) override
	{
		windowPlots[index]->ui.widget->savePng( QString::fromStdString( path ), 0, 0, 3, -1 );
	}
};