#pragma once
#include "stdafx.h"
#include "qcustomplot.h"
#include "Gui/Windows/Plot/WindowPlot.h"
#include "Plot1D.h"

class PlotCSV
{
public:
  static void plot(const std::string &path)
  {
    auto [data, labels] = ParseCSV(path);

    if (labels.empty() || data.empty())
      return;

    std::vector<double> x = data[0];
    std::vector<std::vector<double>> ys = {data.begin() + 1, data.end()};
    std::vector<std::string> ylabels = {labels.begin() + 1, labels.end()};
    std::string plotname = "token count";
    std::string xlabel = labels[0];
    std::string ylabel = plotname;

    Plot1D::plot(x, ys, plotname, xlabel, ylabel, ylabels, Plot::defaultpens, "E:\\Zdeny_PhD_Shenanigans\\articles\\tokens\\plot.png");
  }

private:
  static std::tuple<std::vector<std::vector<double>>, std::vector<std::string>> ParseCSV(const std::string &path)
  {
    std::vector<std::vector<double>> data;
    std::vector<std::string> labels;

    std::ifstream myFile(path);

    if (!myFile.is_open() || !myFile.good())
    {
      LOG_ERROR("Could not open file {}", path);
      return {};
    }

    std::string line, colname;
    double val;

    std::getline(myFile, line);
    std::stringstream ss(line);
    while (std::getline(ss, colname, ','))
      labels.push_back(colname);

    data.resize(labels.size());
    while (std::getline(myFile, line))
    {
      std::stringstream ss(line);
      int col = 0;

      while (ss >> val)
      {
        data[col].push_back(val);

        if (ss.peek() == ',')
          ss.ignore();

        col++;
      }
    }

    return {data, labels};
  }
};
