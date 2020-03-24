#include "stdafx.h"
#include "Plot.h"
#include "Gui/Windows/WindowPlot.h"

std::map<std::string, WindowPlot *> Plot::plots;

QFont Plot::fontTicks( "Newyork", 9 );

QFont Plot::fontLabels( "Newyork", 12 );

std::vector<QPen> Plot::plotPens{ QPen( Qt::blue, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::green, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::darkGreen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::red, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::red, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::lightGray, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) };

QPoint Plot::GetNewPlotPosition( WindowPlot *windowPlot )
{
	int w = 0;
	for ( auto &plot : plots )
		w += plot.second->width();

	int screenwidth = QApplication::desktop()->width();
	int screenheight = QApplication::desktop()->height();

	if ( w + windowPlot->width() < screenwidth )
		return QPoint( w, 0 );
	else
		return QPoint( w % screenwidth, w / screenwidth * windowPlot->height() + 20 );
}
