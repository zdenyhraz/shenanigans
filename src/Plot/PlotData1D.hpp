#pragma once

struct PlotData1D
{
  std::string name;
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
