#pragma once
#include "stdafx.h"
#include "Gui/Windows/Plot/WindowPlot.h"

class Plot
{
public:

	static std::map<std::string, WindowPlot *> plots;
	static QFont fontTicks;
	static QFont fontLabels;
	static std::vector<QPen> plotPens;
	static QPoint GetNewPlotPosition( WindowPlot *windowPlot );

private:

};
