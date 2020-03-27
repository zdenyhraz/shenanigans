#include "stdafx.h"
#include "Plot.h"
#include "Gui/Windows/WindowPlot.h"

std::map<std::string, WindowPlot *> Plot::plots;

QFont Plot::fontTicks( "Newyork", 9 );

QFont Plot::fontLabels( "Newyork", 12 );

std::vector<QPen> Plot::plotPens{ QPen( Qt::blue, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::darkGreen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::red, 0.75, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::red, 0.75, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::lightGray, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) };

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
