#pragma once
#include "stdafx.h"

namespace OptimizationTestFunctions
{
inline double Sphere(const std::vector<double> &arg)
{
  double returnVal = 0;
  for (int i = 0; i < arg.size(); i++)
  {
    returnVal += pow((arg[i]), 2);
  }
  return returnVal;
}

inline double Ackley(const std::vector<double> &arg)
{
  double x = arg[0];
  double y = arg[1];
  return -20 * exp(-0.2 * sqrt(0.5 * (x * x + y * y))) - exp(0.5 * (cos(2 * Constants::Pi * x) + cos(2 * Constants::Pi * y))) + exp(1.) + 20;
}

inline double Himmelblau(const std::vector<double> &arg)
{
  double x = arg[0];
  double y = arg[1];
  return pow(x * x + y - 11, 2) + pow(x + y * y - 7, 2);
}

inline double Rosenbrock(const std::vector<double> &arg)
{
  double x = arg[0];
  double y = arg[1];
  return 100 * pow(y - x * x, 2) + pow(1 - x, 2);
}

inline double Beale(const std::vector<double> &arg)
{
  double x = arg[0];
  double y = arg[1];
  return pow(1.5 - x + x * y, 2) + pow(2.25 - x + x * y * y, 2) + pow(2.625 - x + x * y * y * y, 2);
}
}
