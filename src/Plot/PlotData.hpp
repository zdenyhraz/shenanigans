#pragma once

struct PlotData1D
{
  std::string name;
  std::string savepath;
  size_t location = 0;

  std::vector<double> x;

  std::vector<std::vector<double>> ys;
  std::vector<std::vector<double>> y2s;

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
  double aspectratio = 1.5;
};

struct PlotData2D
{
  std::string name;
  std::string savepath;
  size_t location = 0;

  static constexpr double Default = 0.12345;
  cv::Mat z;
  double xmin = Default;
  double xmax = Default;
  double ymin = Default;
  double ymax = Default;
  double zmin = Default;
  double zmax = Default;
  std::string cmap;
  std::string xlabel;
  std::string ylabel;
  std::string zlabel;
  bool colorbar = true;
  bool interpolate = false;
  bool surf = false;
  double aspectratio = 1;
#ifdef GLAPI
#  include "GLImage.hpp"
  GLImage image;
#endif
};
