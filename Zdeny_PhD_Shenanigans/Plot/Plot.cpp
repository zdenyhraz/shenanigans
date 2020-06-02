#include "stdafx.h"
#include "Plot.h"
#include "Gui/Windows/Plot/WindowPlot.h"

std::map<std::string, WindowPlot *> Plot::plots;

QFont Plot::fontTicks( "Newyork", 9 );
QFont Plot::fontLabels( "Newyork", 12 );
std::vector<QPen> Plot::emptyvectpen{};
QColor Plot::black( 50, 50, 50 );
QColor Plot::green( 0.4660 * 255, 0.6740 * 255, 0.1880 * 255 );
QColor Plot::blue( 0 * 255, 0.4470 * 255, 0.7410 * 255 ); //( 30, 80, 255 )
QColor Plot::red( 0.6350 * 255, 0.0780 * 255, 0.1840 * 255 ); //( 220, 20, 60 )
QColor Plot::orange( 0.8500 * 255, 0.3250 * 255, 0.0980 * 255 ); //( 255, 165, 0 )
QColor Plot::cyan( 0.3010 * 255, 0.7450 * 255, 0.9330 * 255 ); //( 64, 224, 208 )
QColor Plot::magenta( 0.4940 * 255, 0.1840 * 255, 0.5560 * 255 ); //( 150, 0, 150 )

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
