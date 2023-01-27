#pragma once

struct PlotData1D
{
  std::string name;
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
  std::string savepath;
};
