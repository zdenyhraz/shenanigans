#pragma once
#include "stdafx.h"

struct AbstractPlot1D
{
	inline virtual void plot(const std::vector<double>& x, const std::vector<double>& y) = 0;
	inline virtual void plot(const std::vector<double>& x, const std::vector<double>& y1, const std::vector<double>& y2) = 0;
	inline virtual void plot(const std::vector<double>& x, const std::vector<std::vector<double>>& ys) = 0;
	inline virtual void plot(const double x, const double y) = 0;
	inline virtual void plot(const double x, const double y1, const double y2) = 0;
	inline virtual void clear(bool second = false) = 0;
	inline virtual void setAxisNames(std::string xlabel, std::string ylabel) = 0;
	inline virtual void setAxisNames(std::string xlabel, std::string ylabel1, std::string ylabel2) = 0;
	inline virtual void setAxisNames(std::string xlabel, std::string ylabel, std::vector<std::string> ylabels) = 0;
	inline virtual void save(std::string path) = 0;
};

struct AbstractPlot2D
{
	inline virtual void plot(const std::vector<std::vector<double>>& z, std::string xlabel = "x", std::string ylabel = "y", std::string zlabel = "z", double xmin = 0, double xmax = 1, double ymin = 0, double ymax = 1) = 0;
	inline virtual void save(std::string path, int index) = 0;
};
