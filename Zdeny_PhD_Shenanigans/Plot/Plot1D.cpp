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

void Plot1D::plot( const std::vector<std::vector<double>> &z, std::string name, std::string xlabel, std::string ylabel, double xmin, double xmax, std::string savepath )
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






}

