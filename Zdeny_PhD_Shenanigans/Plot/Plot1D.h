#pragma once
#include "stdafx.h"
#include "qcustomplot.h"
#include "Gui/WindowPlot.h"
#include "Plot.h"

// TODO: add addData support
// widget->graph( 0 )->addData( x, y1 );
// widget->graph(1)->addData(x, y2);
using namespace Constants;

class Plot1D
{
public:

	static void CloseAll();

	static void SetupGraph( WindowPlot *windowPlot, int ycnt, int y1cnt, int y2cnt, std::string xlabel, std::string y1label, std::string y2label, std::vector<std::string> &y1names, std::vector<std::string> &y2names );

	// x - y ADD
	static void plot( double x, double y, std::string name, std::string xlabel = "x", std::string ylabel = "y", std::string savepath = emptystring )
	{
		return plotinsides( x, std::vector<double> {y}, emptyvect, name, xlabel, ylabel, emptystring, emptyvectstring, emptyvectstring, savepath );
	}

	// x - y1/y2 ADD
	static void plot( double x, double y1, double y2, std::string name, std::string xlabel = "x", std::string y1label = "y1", std::string y2label = "y2", std::string savepath = emptystring )
	{
		return plotinsides( x, std::vector<double> {y1}, std::vector<double> {y2}, name, xlabel, y1label, y2label, emptyvectstring, emptyvectstring, savepath );
	}

	// x - ys ADD
	static void plot( double x, const std::vector<double> &ys, std::string name, std::string xlabel = "x", std::string ylabel = "y", std::vector<std::string> ynames = emptyvectstring, std::string savepath = emptystring )
	{
		return plotinsides( x, ys, emptyvect, name, xlabel, ylabel, emptystring, ynames, emptyvectstring, savepath );
	}

	// x - y1s/y2s ADD
	static void plot( double x, const std::vector<double> &y1s, const std::vector<double> &y2s, std::string name, std::string xlabel = "x", std::string y1label = "y1", std::string y2label = "y2", std::vector<std::string> y1names = emptyvectstring, std::vector<std::string> y2names = emptyvectstring, std::string savepath = emptystring )
	{
		return plotinsides( x, y1s, y2s, name, xlabel, y1label, y2label, y1names, y2names, savepath );
	}

	// iota - y REPLOT
	static void plot( const std::vector<double> &y, std::string name, std::string xlabel = "x", std::string ylabel = "y", std::string savepath = emptystring )
	{
		std::vector<double> x( y.size() );
		std::iota( x.begin(), x.end(), 0 );
		return plotinsides( x, std::vector<std::vector<double>> {y}, emptyvect2, name, xlabel, ylabel, emptystring, emptyvectstring, emptyvectstring, savepath );
	}

	// x - y REPLOT
	static void plot( const std::vector<double> &x, const std::vector<double> &y, std::string name, std::string xlabel = "x", std::string ylabel = "y", std::string savepath = emptystring )
	{
		return plotinsides( x, std::vector<std::vector<double>> {y}, emptyvect2, name, xlabel, ylabel, emptystring, emptyvectstring, emptyvectstring, savepath );
	}

	// x - y1/y2 REPLOT
	static void plot( const std::vector<double> &x, const std::vector<double> &y1, const std::vector<double> &y2, std::string name, std::string xlabel = "x", std::string y1label = "y1", std::string y2label = "y1", std::string savepath = emptystring )
	{
		return plotinsides( x, std::vector<std::vector<double>> {y1}, std::vector<std::vector<double>> {y2}, name, xlabel, y1label, y2label, emptyvectstring, emptyvectstring, savepath );
	}

	// x - ys REPLOT
	static void plot( const std::vector<double> &x, const std::vector<std::vector<double>> &ys, std::string name, std::string xlabel = "x", std::string ylabel = "y", std::vector<std::string> ynames = emptyvectstring, std::string savepath = emptystring )
	{
		return plotinsides( x, ys, emptyvect2, name, xlabel, ylabel, emptystring, ynames, emptyvectstring, savepath );
	}

	// x - ys1/ys2 REPLOT
	static void plot( const std::vector<double> &x, const std::vector<std::vector<double>> &y1s, const std::vector<std::vector<double>> &y2s, std::string name, std::string xlabel = "x", std::string y1label = "y1", std::string y2label = "y2", std::vector<std::string> y1names = emptyvectstring, std::vector<std::string> y2names = emptyvectstring, std::string savepath = emptystring )
	{
		return plotinsides( x, y1s, y2s, name, xlabel, y1label, y2label, y1names, y2names, savepath );
	}

	// core REPLOT
	static void plotinsides( const std::vector<double> &x, const std::vector<std::vector<double>> &y1s, const std::vector<std::vector<double>> &y2s, std::string name, std::string xlabel = "x", std::string y1label = "y1", std::string y2label = "y2", std::vector<std::string> y1names = emptyvectstring, std::vector<std::string> y2names = emptyvectstring, std::string savepath = emptystring );

	// core ADD
	static void plotinsides( double x, const std::vector<double> &y1s, const std::vector<double> &y2s, std::string name, std::string xlabel = "x", std::string y1label = "y1", std::string y2label = "y2", std::vector<std::string> y1names = emptyvectstring, std::vector<std::string> y2names = emptyvectstring, std::string savepath = emptystring );

private:
	static std::map<std::string, WindowPlot *> plots;

	static std::function<void( std::string )> OnClose;
};