#pragma once
#include "stdafx.h"

namespace OptimizationTestFunctions
{
inline double Sphere(const std::vector<double>& arg)
{
  double returnVal = 0;
  for (int i = 0; i < arg.size(); i++)
  {
    returnVal += sqr(arg[i]);
  }
  return returnVal;
}

inline double Ackley(const std::vector<double>& arg)
{
  double x = arg[0];
  double y = arg[1];
  return -20 * exp(-0.2 * sqrt(0.5 * (x * x + y * y))) - exp(0.5 * (cos(2 * Constants::Pi * x) + cos(2 * Constants::Pi * y))) + exp(1.) + 20;
}

inline double Himmelblau(const std::vector<double>& arg)
{
  double x = arg[0];
  double y = arg[1];
  return sqr(x * x + y - 11) + sqr(x + y * y - 7);
}

inline double Rosenbrock(const std::vector<double>& arg)
{
  double x = arg[0];
  double y = arg[1];
  return 100 * sqr(y - x * x) + sqr(1 - x);
}

inline double Beale(const std::vector<double>& arg)
{
  double x = arg[0];
  double y = arg[1];
  return sqr(1.5 - x + x * y) + sqr(2.25 - x + x * y * y) + sqr(2.625 - x + x * y * y * y);
}

inline double Paraboloid(const std::vector<double>& arg)
{
  double val = 0;
  for (int i = 0; i < arg.size(); i++)
    val += sqr(arg[i] - i - 1);
  return val;
}
}
