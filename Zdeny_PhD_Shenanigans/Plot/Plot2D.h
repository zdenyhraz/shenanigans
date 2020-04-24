#pragma once
#include "stdafx.h"
#include "qcustomplot.h"
#include "Gui/Windows/Plot/WindowPlot.h"
#include "Plot.h"
#include "Utils/vectmat.h"

class Plot2D
{
public:

	static void CloseAll();

	static void SetupGraph( WindowPlot *windowPlot, std::string xlabel, std::string ylabel, std::string zlabel );

	static void plot( const Mat &z, std::string name )
	{
		return plotcore( matToVect2( z ), name, "x", "y", name );
	}

	static void plot( const Mat &z, std::string name, std::string xlabel, std::string ylabel, std::string zlabel, double xmin = 0, double xmax = 1, double ymin = 0, double ymax = 1, double colRowRatio = 1, std::string savepath = "" )
	{
		return plotcore( matToVect2( z ), name, xlabel, ylabel, zlabel, xmin, xmax, ymin, ymax, colRowRatio, savepath );
	}

	static void plot( const std::vector<std::vector<double>> &z, std::string name, std::string xlabel = "x", std::string ylabel = "y", std::string zlabel = "z", double xmin = 0, double xmax = 1, double ymin = 0, double ymax = 1, double colRowRatio = 1, std::string savepath = "" )
	{
		return plotcore( z, name, xlabel, ylabel, zlabel, xmin, xmax, ymin, ymax, colRowRatio, savepath );
	}

	static void plotcore( const std::vector<std::vector<double>> &z, std::string name, std::string xlabel = "x", std::string ylabel = "y", std::string zlabel = "z", double xmin = 0, double xmax = 1, double ymin = 0, double ymax = 1, double colRowRatio = 1, std::string savepath = "" );

private:

	static std::function<void( std::string )> OnClose;
};