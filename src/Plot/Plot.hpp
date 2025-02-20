#pragma once
// #include "CvPlot.hpp"
// #include "PyPlot.hpp"

#ifdef GLAPI
#  include "ImGuiPlot.hpp"
using Plot = ImGuiPlot;
#else
using Plot = CvPlot;
#endif
