#include "stdafx.h"
#include "plotterqt2D.h"

std::map<std::string, WindowPlot *> Plot2D::plots;

std::function<void( std::string )> Plot2D::OnClose = []( std::string name )
{
	auto idx = plots.find( name );
	if ( idx != plots.end() )
	{
		LOG_DEBUG( "2Dplot named '{}' found in plot registry, deleting plot", name );
		delete idx->second;
		plots.erase( idx );
	}
	else
	{
		LOG_DEBUG( "2Dplot named '{}' not found in plot registry, not deleting plot", name );
	}
};