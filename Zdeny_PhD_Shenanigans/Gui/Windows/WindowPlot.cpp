#include "stdafx.h"
#include "WindowPlot.h"

WindowPlot::WindowPlot( std::string name, double rowColRatio, std::function<void( std::string )> &OnClose ) : QMainWindow(), name( name ), OnClose( OnClose )
{
	ui.setupUi( this );
	int wRows = 400;
	int wCols = ( double )wRows / rowColRatio + 150;
	ui.widget->setFixedSize( wCols, wRows );
	setFixedSize( ui.widget->width(), ui.widget->height() + 20 );
}

WindowPlot::~WindowPlot()
{
}

void WindowPlot::closeEvent( QCloseEvent *event )
{
	OnClose( name );
}