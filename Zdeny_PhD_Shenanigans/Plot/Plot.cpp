#include "stdafx.h"
#include "Plot.h"
#include "Gui/Windows/WindowPlot.h"

std::map<std::string, WindowPlot *> Plot::plots;

QFont Plot::fontTicks( "Newyork", 9 );

QFont Plot::fontLabels( "Newyork", 12 );

std::vector<QPen> Plot::plotPens{ QPen( Qt::blue, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::green, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::red, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::red, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::lightGray, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) };

QPoint Plot::GetNewPlotPosition( WindowPlot *windowPlot )
{
	int plotcnt = plots.size();
	int plotwidth = windowPlot->width();
	int plotheight = windowPlot->height();
	int screenwidth = QApplication::desktop()->width();
	int screenheight = QApplication::desktop()->height();

	int plotsPerRow = screenwidth / plotwidth;

	if ( plotcnt < plotsPerRow )
		return QPoint( plotcnt * plotwidth, 0 );
	else
		return QPoint( ( plotcnt - plotsPerRow ) * plotwidth, plotheight + 20 );
}
