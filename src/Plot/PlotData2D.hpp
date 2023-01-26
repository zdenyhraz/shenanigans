#pragma once

enum PlotColormap
{
  Jet = ImPlotColormap_Jet,
  Gray = ImPlotColormap_Greys
};

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
  PlotColormap cmap = Jet;
  std::string xlabel = "x";
  std::string ylabel = "y";
  bool colorbar = true;
};
