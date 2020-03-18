#include "stdafx.h"
#include "Plot.h"
#include "Gui/WindowPlot.h"

std::map<std::string, WindowPlot *> Plot::plots;

QFont Plot::fontTicks( "Newyork", 9 );

QFont Plot::fontLabels( "Newyork", 12 );

std::vector<QPen> Plot::plotPens{ QPen( Qt::gray, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::blue, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::green, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ), QPen( Qt::red, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) };

QPoint Plot::GetNewPlotPosition( WindowPlot *windowPlot )
{
	return QPoint( plots.size() * windowPlot->width(), 0 );
}