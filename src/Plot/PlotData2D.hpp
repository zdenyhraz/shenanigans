#pragma once
#include "GLImage.hpp"

struct PlotData2D
{
  std::string name;
  cv::Mat z;
  f64 xmin = 0;
  f64 xmax = 1;
  f64 ymin = 0;
  f64 ymax = 1;
  f64 zmin = 0;
  f64 zmax = 1;
  std::string cmap = "jet";
  std::string xlabel = "x";
  std::string ylabel = "y";
  std::string zlabel = "z";
  bool colorbar = true;
  bool interpolate = false;
  bool surf = false;
  f64 aspectratio = 1;
  std::string savepath;
  GLImage image;
};
