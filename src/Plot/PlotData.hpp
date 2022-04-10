#pragma once

struct PlotData1D
{
  std::vector<f64> x;
  std::vector<std::vector<f64>> ys;
  std::vector<std::vector<f64>> y2s;
  std::vector<std::string> ylabels;
  std::vector<std::string> y2labels;
  std::string xlabel = "x";
  std::string ylabel = "y";
  std::string y2label = "y2";
  bool log = false;
};

enum PlotColormap
{
  Jet = ImPlotColormap_Jet,
  Gray = ImPlotColormap_Greys
};

struct PlotData2D
{
  cv::Mat z;
  f64 xmin = 0;
  f64 xmax = 1;
  f64 ymin = 0;
  f64 ymax = 1;
  f64 zmin = 0;
  f64 zmax = 1;
  PlotColormap cmap = Jet;
  std::string xlabel;
  std::string ylabel;
};
