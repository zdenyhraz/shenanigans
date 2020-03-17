#pragma once
#include "stdafx.h"
#include "qcustomplot.h"
#include "Gui/WindowPlot.h"
#include "Plot.h"

class Plot1D
{
public:

	static void plot( const std::vector<double> &x, const std::vector<double> &y, std::string name, std::string xlabel = "x", std::string ylabel = "y", double xmin = 0, double xmax = 1, std::string savepath = "" )
	{
		return plotinsides( x, std::vector<std::vector<double>> {y}, std::vector<std::vector<double>> {}, name, xlabel, ylabel, ylabel, xmin, xmax, savepath );
	}

	static void plot( const std::vector<double> &y, std::string name, std::string xlabel = "x", std::string ylabel = "y", double xmin = 0, double xmax = 1, std::string savepath = "" )
	{
		std::vector<double> x( y.size() );
		std::iota( x.begin(), x.end(), 0 );
		return plotinsides( x, std::vector<std::vector<double>> {y}, std::vector<std::vector<double>> {}, name, xlabel, ylabel, ylabel, xmin, xmax, savepath );
	}

	static void plot( const std::vector<double> &x, const std::vector<std::vector<double>> &y1s, const std::vector<std::vector<double>> &y2s, std::string name, std::string xlabel = "x", std::string y1label = "y1", std::string y2label = "y2", double xmin = 0, double xmax = 1, std::string savepath = "" )
	{
		return plotinsides( x, y1s, y2s, name, xlabel, y1label, y2label, xmin, xmax, savepath );
	}

	static void plotinsides( const std::vector<double> &x, const std::vector<std::vector<double>> &y1s, const std::vector<std::vector<double>> &y2s, std::string name, std::string xlabel = "x", std::string y1label = "y1", std::string y2label = "y2", double xmin = 0, double xmax = 1, std::string savepath = "" );

private:
	static std::map<std::string, WindowPlot *> plots;

	static std::function<void( std::string )> OnClose;
};