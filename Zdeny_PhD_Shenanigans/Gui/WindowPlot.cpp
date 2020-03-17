#include "stdafx.h"
#include "WindowPlot.h"

WindowPlot::WindowPlot( std::string name, std::function<void( std::string )> &OnClose ) : QMainWindow(), name( name ), OnClose( OnClose )
{
	ui.setupUi( this );
}

WindowPlot::~WindowPlot()
{
}

void WindowPlot::Clear()
{
	ui.widget->clearGraphs();
	ui.widget->clearPlottables();
	ui.widget->clearItems();

	delete colorMap;
	delete colorScale;
	delete marginGroup;
}

void WindowPlot::closeEvent( QCloseEvent *event )
{
	OnClose( name );
}