#include "stdafx.h"
#include "WindowPlot.h"

WindowPlot::WindowPlot( std::function<void( WindowPlot * )> &OnClose ) : QMainWindow(), OnClose( OnClose )
{
	ui.setupUi( this );
}

WindowPlot::~WindowPlot()
{
}

void WindowPlot::closeEvent( QCloseEvent *event )
{
	OnClose( this );
}