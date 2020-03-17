#pragma once
#include "stdafx.h"
#include "qcustomplot.h"
#include "plotter.h"
#include "Gui/WindowPlot.h"
#include "Plot.h"

class Plot2D
{
public:

	static void plot( const Mat &z, std::string name, std::string xlabel = "x", std::string ylabel = "y", std::string zlabel = "z", double xmin = 0, double xmax = 1, double ymin = 0, double ymax = 1, std::string savepath = "" )
	{
		return plot( matToVect2( z ), name, xlabel, ylabel, zlabel, xmin, xmax, ymin, ymax, savepath );
	}

	static void plot( const std::vector<std::vector<double>> &z, std::string name, std::string xlabel = "x", std::string ylabel = "y", std::string zlabel = "z", double xmin = 0, double xmax = 1, double ymin = 0, double ymax = 1, std::string savepath = "" );

private:
	static std::map<std::string, WindowPlot *> plots;

	static std::function<void( std::string )> OnClose;
};