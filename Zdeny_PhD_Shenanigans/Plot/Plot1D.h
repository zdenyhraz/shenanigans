#pragma once
#include "stdafx.h"
#include "qcustomplot.h"
#include "Gui/WindowPlot.h"
#include "Plot.h"

class Plot1D
{
public:

	static void plot( const std::vector<std::vector<double>> &z, std::string name, std::string xlabel = "x", std::string ylabel = "y", double xmin = 0, double xmax = 1, std::string savepath = "" );

private:
	static std::map<std::string, WindowPlot *> plots;

	static std::function<void( std::string )> OnClose;
};