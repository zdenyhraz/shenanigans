#pragma once

namespace OptimizationTestFunctions
{
inline double Cone(const std::vector<double>& arg)
{
  double returnVal = 0;
  for (size_t i = 0; i < arg.size(); i++)
    returnVal += std::abs(arg[i]);
  return returnVal;
}

inline double Paraboloid(const std::vector<double>& arg)
{
  double val = 0;
  for (size_t i = 0; i < arg.size(); i++)
    val += sqr(arg[i] - i - 1);
  return val;
}

inline double Rastigrin(const std::vector<double>& arg)
{
  double retval = 0;
  for (const auto& x : arg)
    retval += x * x - 10. * std::cos(2. * Constants::Pi * x);
  return retval;
}

inline double Ackley(const std::vector<double>& arg)
{
  double x = arg[0];
  double y = arg[1];
  return -20. * exp(-0.2 * sqrt(0.5 * (x * x + y * y))) - exp(0.5 * (cos(2 * Constants::Pi * x) + cos(2 * Constants::Pi * y))) + exp(1.) + 20;
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
  static constexpr double a = 1;
  static constexpr double b = 100;
  return sqr(a - x) + b * sqr(y - sqr(x));
}

inline double Beale(const std::vector<double>& arg)
{
  double x = arg[0];
  double y = arg[1];
  return sqr(1.5 - x + x * y) + sqr(2.25 - x + x * y * y) + sqr(2.625 - x + x * y * y * y);
}

inline double Goldstein(const std::vector<double>& arg)
{
  double x = arg[0];
  double y = arg[1];
  return (1 + sqr(x + y + 1) * (19 - 14 * x + 3 * x * x - 14 * y + 6 * x * y + 3 * y * y)) * (30 + sqr(2 * x - 3 * y) * (18 - 32 * x + 12 * x * x + 48 * y - 36 * x * y + 27 * y * y));
}

inline double Bukin(const std::vector<double>& arg)
{
  double x = arg[0];
  double y = arg[1];
  return 100 * std::sqrt(std::abs(y - 0.01 * x * x)) + 0.01 * std::abs(x + 10);
}

inline double Levi(const std::vector<double>& arg)
{
  double x = arg[0];
  double y = arg[1];
  double pi = Constants::Pi;
  return sqr(std::sin(3. * pi * x)) + sqr(x - 1) * (1 + sqr(std::sin(3 * pi * y))) + sqr(y - 1) * (1 + sqr(std::sin(2 * pi * y)));
}

inline double Camel(const std::vector<double>& arg)
{
  double x = arg[0];
  double y = arg[1];
  return 2. * x * x - 1.05 * std::pow(x, 4) + std::pow(x, 6) / 6 + x * y + y * y;
}

inline double Easom(const std::vector<double>& arg)
{
  double x = arg[0];
  double y = arg[1];
  double pi = Constants::Pi;
  return -std::cos(x) * std::cos(y) * std::exp(-(sqr(x - pi) + sqr(y - pi)));
}

inline double CrossInTray(const std::vector<double>& arg)
{
  double x = arg[0];
  double y = arg[1];
  double pi = Constants::Pi;
  return -0.0001 * std::pow(std::abs(std::sin(x) * std::sin(y) * std::exp(std::abs(100 - std::sqrt(x * x + y * y) / pi))) + 1, 0.1);
}

inline double HolderTable(const std::vector<double>& arg)
{
  double x = arg[0];
  double y = arg[1];
  return -std::abs(std::sin(x) * std::cos(y) * std::exp(std::abs(1 - std::sqrt(x * x + y * y) / Constants::Pi)));
}

inline double McCormick(const std::vector<double>& arg)
{
  double x = arg[0];
  double y = arg[1];
  return std::sin(x + y) + sqr(x - y) - 1.5 * x + 2.5 * y + 1;
}

inline double Schaffer(const std::vector<double>& arg)
{
  double x = arg[0];
  double y = arg[1];
  return 0.5 + (sqr(std::sin(x * x - y * y)) - 0.5) / sqr(1 + 0.001 * (x * x + y * y));
}

inline double Styblinski(const std::vector<double>& arg)
{
  double retval = 0;
  for (const auto& x : arg)
    retval += (std::pow(x, 4) - 16 * x * x + 5 * x) / 2;
  return retval;
}

inline double Exp(const std::vector<double>& arg)
{
  double retval = 0;
  for (size_t i = 0; i < arg.size(); i++)
    retval += sqr((i + 1) * arg[i] - 1 - i);
  return std::exp(std::sqrt(retval));
}

inline double Noisy(const std::vector<double>& arg)
{
  double retval = 0;
  for (size_t i = 0; i < arg.size(); i++)
    retval += arg[i] * (std::sin(3 * arg[i]) + 0.3 * rand11());
  return retval / arg.size();
}
}
