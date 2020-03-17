#pragma once
#include "stdafx.h"
#include "qcustomplot.h"
#include "plotter.h"
#include "Gui/WindowPlot.h"
#include "plotterqt.h"

class Plot2D
{
public:

	inline static void plot( const Mat &z, std::string name, std::string xlabel = "x", std::string ylabel = "y", std::string zlabel = "z", double xmin = 0, double xmax = 1, double ymin = 0, double ymax = 1, std::string savepath = "" );

	inline static void plot( const std::vector<std::vector<double>> &z, std::string name, std::string xlabel = "x", std::string ylabel = "y", std::string zlabel = "z", double xmin = 0, double xmax = 1, double ymin = 0, double ymax = 1, std::string savepath = "" );

private:
	static std::map<std::string, WindowPlot *> plots;

	static std::function<void( std::string )> OnClose;
};