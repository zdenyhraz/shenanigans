#pragma once
#include "stdafx.h"

struct AbstractPlot1D
{
	inline virtual void plot(const std::vector<double>& x, const std::vector<double>& y) = 0;
	inline virtual void plot(const std::vector<double>& x, const std::vector<double>& y1, const std::vector<double>& y2) = 0;
	inline virtual void plot(const double x, const double y) = 0;
	inline virtual void plot(const double x, const double y1, const double y2) = 0;
	inline virtual void clear(bool second) = 0;
	inline virtual void setAxisNames(std::string xlabel, std::string ylabel) = 0;
	inline virtual void setAxisNames(std::string xlabel, std::string ylabel1, std::string ylabel2) = 0;
	inline virtual void save(std::string path) = 0;
};

struct AbstractPlot2D
{
	inline virtual void plot(const std::vector<std::vector<double>>& z) = 0;
	inline virtual void setAxisNames(std::string xlabel, std::string ylabel, std::string zlabel) = 0;
	inline virtual void save(std::string path) = 0;
	inline virtual void setSize(int nx, int ny) = 0;
	inline virtual void setRange(double xmin, double xmax, double ymin, double ymax) = 0;
};
