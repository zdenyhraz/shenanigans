#pragma once

#ifdef GLAPI
#  include "ImGuiPlot.hpp"
using Plot = ImGuiPlot;
#else
#  include "CvPlot.hpp"
using Plot = CvPlot;
#endif
