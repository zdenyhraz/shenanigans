#pragma once
#include "stdafx.h"
#include "qcustomplot.h"
#include "Gui/WindowPlot.h"
#include "Plot.h"

class Plot1D
{
public:

	static void plot( const std::vector<double> &y, std::string name, std::string xlabel = "x", std::string ylabel = "y", std::string savepath = "" )
	{
		std::vector<double> x( y.size() );
		std::iota( x.begin(), x.end(), 0 );
		return plotinsides( x, std::vector<std::vector<double>> {y}, std::vector<std::vector<double>> {}, name, xlabel, ylabel, ylabel, std::vector<std::string> {name}, std::vector<std::string> {}, savepath );
	}

	static void plot( const std::vector<double> &x, const std::vector<double> &y, std::string name, std::string xlabel = "x", std::string ylabel = "y", std::string savepath = "" )
	{
		return plotinsides( x, std::vector<std::vector<double>> {y}, std::vector<std::vector<double>> {}, name, xlabel, ylabel, ylabel, std::vector<std::string> {name}, std::vector<std::string> {}, savepath );
	}

	static void plot( const std::vector<double> &x, const std::vector<std::vector<double>> &ys, std::string name, std::string xlabel = "x", std::string ylabel = "y", std::vector<std::string> &ynames = std::vector<std::string> {}, std::string savepath = "" )
	{
		return plotinsides( x, ys, std::vector<std::vector<double>> {}, name, xlabel, ylabel, ylabel, ynames, std::vector<std::string> {}, savepath );
	}

	static void plot( const std::vector<double> &x, const std::vector<std::vector<double>> &y1s, const std::vector<std::vector<double>> &y2s, std::string name, std::string xlabel = "x", std::string y1label = "y1", std::string y2label = "y2", std::vector<std::string> &y1names = std::vector<std::string> {}, std::vector<std::string> &y2names = std::vector<std::string> {}, std::string savepath = "" )
	{
		return plotinsides( x, y1s, y2s, name, xlabel, y1label, y2label, y1names, y2names, savepath );
	}

	static void plotinsides( const std::vector<double> &x, const std::vector<std::vector<double>> &y1s, const std::vector<std::vector<double>> &y2s, std::string name, std::string xlabel = "x", std::string y1label = "y1", std::string y2label = "y2", std::vector<std::string> &y1names = std::vector<std::string> {}, std::vector<std::string> &y2names = std::vector<std::string> {}, std::string savepath = "" );

private:
	static std::map<std::string, WindowPlot *> plots;

	static std::function<void( std::string )> OnClose;
};