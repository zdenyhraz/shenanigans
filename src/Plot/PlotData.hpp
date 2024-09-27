#pragma once

struct PlotData1D
{
  std::string name;
  std::string savepath;
  usize location = 0;

  std::vector<f64> x;

  std::vector<std::vector<f64>> ys;
  std::vector<std::vector<f64>> y2s;

  std::vector<std::string> ylabels;
  std::vector<std::string> y2labels;

  std::vector<std::string> ycolors;
  std::vector<std::string> y2colors;

  std::vector<std::string> ylinestyles;
  std::vector<std::string> y2linestyles;

  std::string xlabel = "x";
  std::string ylabel = "y";
  std::string y2label = "y2";
  bool log = false;
  f64 aspectratio = 1.5;
};

struct PlotData2D
{
  std::string name;
  std::string savepath;
  usize location = 0;

  static constexpr f64 Default = 0.12345;
  cv::Mat z;
  f64 xmin = Default;
  f64 xmax = Default;
  f64 ymin = Default;
  f64 ymax = Default;
  f64 zmin = Default;
  f64 zmax = Default;
  std::string cmap;
  std::string xlabel;
  std::string ylabel;
  std::string zlabel;
  bool colorbar = true;
  bool interpolate = false;
  bool surf = false;
  f64 aspectratio = 1;
#ifdef GLAPI
  #include "GLImage.hpp"
  GLImage image;
#endif
};
