#include "stdafx.h"
#include "Plot2D.h"

std::map<std::string, WindowPlot *> Plot2D::plots;

std::function<void( std::string )> Plot2D::OnClose = []( std::string name )
{
	auto idx = plots.find( name );
	if ( idx != plots.end() )
	{
		LOG_DEBUG( "Deleting 2Dplot '{}'", name );
		delete idx->second;
		plots.erase( idx );
	}
	else
	{
		LOG_ERROR( "2Dplot '{}' not found in plot registry (why tho?), not deleting", name );
	}
};

void Plot2D::CloseAll()
{
	for ( auto &plt : plots )
	{
		delete plt.second;
		plots.erase( plt.first );
	}
}

void Plot2D::plot( const std::vector<std::vector<double>> &z, std::string name, std::string xlabel, std::string ylabel, std::string zlabel, double xmin, double xmax, double ymin, double ymax, std::string savepath )
{
	WindowPlot *windowPlot;
	auto idx = plots.find( name );
	if ( idx != plots.end() )
	{
		LOG_DEBUG( "Updating 2Dplot '{}'", name );
		windowPlot = idx->second;
		windowPlot->ui.widget->graph( 0 )->data().clear();
	}
	else
	{
		LOG_DEBUG( "Creating 2Dplot '{}'", name );
		windowPlot = new WindowPlot( name, OnClose );
		plots[name] = windowPlot;
		SetupGraph( windowPlot, z[0].size(), z.size(), xlabel, ylabel, zlabel, xmin, xmax, ymin, ymax );
	}

	for ( int xIndex = 0; xIndex < z[0].size(); ++xIndex )
		for ( int yIndex = 0; yIndex < z.size(); ++yIndex )
			windowPlot->colorMap->data()->setCell( xIndex, yIndex, z[yIndex][xIndex] );

	windowPlot->colorMap->rescaleDataRange();
	windowPlot->ui.widget->rescaleAxes();
	windowPlot->colorScale->rescaleDataRange( 0 );
	windowPlot->ui.widget->replot();
	windowPlot->show();

	if ( savepath != "" )
		windowPlot->ui.widget->savePng( QString::fromStdString( savepath ), 0, 0, 3, -1 );
}

void Plot2D::SetupGraph( WindowPlot *windowPlot, int sizex, int sizey, std::string xlabel, std::string ylabel, std::string zlabel, double xmin, double xmax, double ymin, double ymax )
{
	windowPlot->ui.widget->addGraph();//create graph
	windowPlot->ui.widget->axisRect()->setupFullAxesBox( true ); //configure axis rect
	windowPlot->ui.widget->xAxis->setLabel( QString::fromStdString( xlabel ) ); //give the axes some labels
	windowPlot->ui.widget->yAxis->setLabel( QString::fromStdString( ylabel ) ); //give the axes some labels
	windowPlot->colorMap = new QCPColorMap( windowPlot->ui.widget->xAxis, windowPlot->ui.widget->yAxis ); //set up the QCPColorMap
	windowPlot->colorMap->data()->setSize( sizex, sizey ); //we want the color map to have nx * ny data points
	windowPlot->colorMap->data()->setRange( QCPRange( xmin, xmax ), QCPRange( ymin, ymax ) ); //and span the coordinate range in both key (x) and value (y) dimensions
	windowPlot->ui.widget->setInteractions( QCP::iRangeDrag | QCP::iRangeZoom ); //allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking
	windowPlot->colorScale = new QCPColorScale( windowPlot->ui.widget ); //add a color scale
	windowPlot->ui.widget->plotLayout()->addElement( 0, 1, windowPlot->colorScale ); //add it to the right of the main axis rect
	windowPlot->colorScale->setType( QCPAxis::atRight ); //scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
	windowPlot->colorMap->setColorScale( windowPlot->colorScale ); //associate the color map with the color scale
	windowPlot->colorScale->axis()->setLabel( QString::fromStdString( zlabel ) ); //add the z value name
	windowPlot->colorMap->setGradient( QCPColorGradient::gpJet ); //set the color gradient of the color map to one of the presets
	windowPlot->marginGroup = new QCPMarginGroup( windowPlot->ui.widget ); //make sure the axis rect and color scale synchronize their bottom and top margins (so they line up)
	windowPlot->ui.widget->axisRect()->setMarginGroup( QCP::msBottom | QCP::msTop, windowPlot->marginGroup ); //align plot
	windowPlot->colorScale->setMarginGroup( QCP::msBottom | QCP::msTop, windowPlot->marginGroup ); //align colorbar
	windowPlot->colorMap->setInterpolate( true ); //interpolate bro
	windowPlot->ui.widget->xAxis->setTickLabelFont( fontTicks );
	windowPlot->ui.widget->yAxis->setTickLabelFont( fontTicks );
	windowPlot->colorScale->axis()->setTickLabelFont( fontTicks );
	windowPlot->ui.widget->xAxis->setLabelFont( fontLabels );
	windowPlot->ui.widget->yAxis->setLabelFont( fontLabels );
	windowPlot->ui.widget->yAxis->setRangeReversed( true );
	windowPlot->colorScale->axis()->setLabelFont( fontLabels );
}


