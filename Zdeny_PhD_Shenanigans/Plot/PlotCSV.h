#pragma once
#include "stdafx.h"
#include "Plot1D.h"

class PlotCSV
{
public:
  static void plot(const std::string &path, const std::string &savepath = "");

private:
  static std::tuple<std::vector<double>, std::vector<std::vector<double>>, std::string, std::vector<std::string>> ParseCSV(const std::string &path);

  static int counter;
};
