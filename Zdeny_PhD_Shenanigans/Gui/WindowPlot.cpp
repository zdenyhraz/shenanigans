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
	ui.widget->clearItems();
	ui.widget->clearGraphs();
	ui.widget->clearPlottables();
}

void WindowPlot::closeEvent( QCloseEvent *event )
{
	OnClose( name );
}