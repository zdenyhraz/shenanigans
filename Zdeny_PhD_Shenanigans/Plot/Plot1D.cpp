#include "stdafx.h"
#include "Plot1D.h"

std::map<std::string, WindowPlot *> Plot1D::plots;

std::function<void( std::string )> Plot1D::OnClose = []( std::string name )
{
	auto idx = plots.find( name );
	if ( idx != plots.end() )
	{
		LOG_DEBUG( "Deleting 1Dplot '{}'", name );
		delete idx->second;
		plots.erase( idx );
	}
	else
	{
		LOG_ERROR( "1Dplot '{}' not found in plot registry (why tho?), not deleting", name );
	}
};

void Plot1D::plot( const std::vector<double> &y, std::string name, std::string xlabel, std::string ylabel, double xmin, double xmax, std::string savepath )
{
	WindowPlot *windowPlot;

	auto idx = plots.find( name );
	if ( idx != plots.end() )
	{
		LOG_DEBUG( "Updating 1Dplot '{}'", name );
		windowPlot = idx->second;
		windowPlot->Clear();
	}
	else
	{
		LOG_DEBUG( "Creating 1Dplot '{}'", name );
		windowPlot = new WindowPlot( name, OnClose );
	}
	plots[name] = windowPlot;

	windowPlot->ui.widget->addGraph();
	windowPlot->ui.widget->xAxis->setTickLabelFont( fontTicks );
	windowPlot->ui.widget->yAxis->setTickLabelFont( fontTicks );
	windowPlot->ui.widget->xAxis->setLabelFont( fontLabels );
	windowPlot->ui.widget->yAxis->setLabelFont( fontLabels );
	windowPlot->ui.widget->graph( 0 )->setPen( plotPen );
	windowPlot->ui.widget->setInteractions( QCP::iRangeDrag | QCP::iRangeZoom );
	windowPlot->ui.widget->xAxis->setLabel( QString::fromStdString( xlabel ) );
	windowPlot->ui.widget->yAxis->setLabel( QString::fromStdString( ylabel ) );

	std::vector<double> iotam( y.size() );
	std::iota( iotam.begin(), iotam.end(), 0 );
	windowPlot->ui.widget->graph( 0 )->setData( QVector<double>::fromStdVector( iotam ), QVector<double>::fromStdVector( y ) );
	windowPlot->ui.widget->rescaleAxes();
	windowPlot->ui.widget->replot();
	windowPlot->show();

	if ( savepath != "" )
	{
		windowPlot->ui.widget->savePng( QString::fromStdString( savepath ), 0, 0, 3, -1 );
	}
}

