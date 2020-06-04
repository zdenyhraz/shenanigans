#include "stdafx.h"
#include "Plot.h"
#include "Gui/Windows/Plot/WindowPlot.h"

std::map<std::string, WindowPlot *> Plot::plots;

QFont Plot::fontTicks( "Newyork", 13 );//9
QFont Plot::fontLabels( "Newyork", 17 );//12
std::vector<QPen> Plot::emptyvectpen{};
QColor Plot::black( 50, 50, 50 );
QColor Plot::green( 119, 182, 48 );
QColor Plot::blue( 30, 80, 255 );
QColor Plot::red( 220, 20, 60 );
QColor Plot::orange( 255, 165, 0 );
QColor Plot::cyan( 64, 224, 208 );
QColor Plot::magenta( 150, 0, 150 );

QPoint Plot::GetNewPlotPosition( WindowPlot *windowPlot )
{
	int w = 0;
	int h = 0;
	for ( auto &plot : plots )
	{
		if ( w + plot.second->width() > QApplication::desktop()->width() )
		{
			w = plot.second->width();
			h += plot.second->height() + 25;
			if ( h  > QApplication::desktop()->height() )
				h = 0;
		}
		else
		{
			w += plot.second->width();
		}
	}

	if ( w + windowPlot->width() > QApplication::desktop()->width() )
	{
		w = 0;
		h += windowPlot->height() + 25;
		if ( h > QApplication::desktop()->height() )
			h = 0;
	}

	return QPoint( w, h );
}
