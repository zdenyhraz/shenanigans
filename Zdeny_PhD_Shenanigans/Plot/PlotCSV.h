#pragma once
#include "stdafx.h"
#include "Plot1D.h"

class PlotCSV
{
public:
  static void plot(const std::string &path)
  {
    auto [data, labels] = ParseCSV(path);

    if (data.empty() || labels.empty())
      return;

    std::string plotname = "token count";
    std::vector<double> x = data[0];
    std::vector<std::vector<double>> ys = {data.begin() + 1, data.end()};
    std::vector<std::string> ylabels = {labels.begin() + 1, labels.end()};
    std::string xlabel = labels[0];
    std::string ylabel = plotname;

    Plot1D::plot(x, ys, plotname, xlabel, ylabel, ylabels, Plot::defaultpens, "E:\\Zdeny_PhD_Shenanigans\\articles\\tokens\\plot.png");
  }

private:
  static std::tuple<std::vector<std::vector<double>>, std::vector<std::string>> ParseCSV(const std::string &path)
  {
    std::ifstream file(path);
    if (!file.is_open() || !file.good())
    {
      LOG_ERROR("Could not open file {}", path);
      return {};
    }

    std::vector<std::vector<double>> data;
    std::vector<std::string> labels;
    std::string line, colname;
    double value;

    // fill label vector from first line
    std::getline(file, line);
    std::stringstream ss(line);
    while (std::getline(ss, colname, ','))
      labels.push_back(colname);

    // the data dimension is the same as the label dimension
    data.resize(labels.size());

    while (std::getline(file, line))
    {
      std::stringstream ss(line);
      int col = 0;

      while (ss >> value)
      {
        data[col].push_back(value);

        if (ss.peek() == ',')
          ss.ignore();

        col++;
      }
    }

    return {data, labels};
  }
};
