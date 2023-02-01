#pragma once
#include "GLImage.hpp"

struct PlotData2D
{
  static constexpr f64 Default = 0.12345;
  std::string name;
  cv::Mat z;
  f64 xmin = Default;
  f64 xmax = Default;
  f64 ymin = Default;
  f64 ymax = Default;
  f64 zmin = Default;
  f64 zmax = Default;
  std::string cmap = "jet";
  std::string xlabel;
  std::string ylabel;
  std::string zlabel;
  bool colorbar = true;
  bool interpolate = false;
  bool surf = false;
  f64 aspectratio = 1;
  std::string savepath;
  GLImage image;
};
