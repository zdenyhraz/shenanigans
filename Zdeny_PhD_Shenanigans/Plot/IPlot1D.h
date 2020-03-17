#pragma once
#include "stdafx.h"

struct IPlot1D
{
	// x - y
	inline virtual void plot( const double x, const double y ) = 0;

	// x - y1/y2
	inline virtual void plot( const std::vector<double> &x, const std::vector<double> &y1, const std::vector<double> &y2 ) = 0;
	inline virtual void plot( const double x, const double y1, const double y2 ) = 0;


};
