#pragma once

namespace OptimizationTestFunctions
{
inline f64 Cone(const std::vector<f64>& arg)
{
  f64 returnVal = 0;
  for (usize i = 0; i < arg.size(); i++)
    returnVal += std::abs(arg[i]);
  return returnVal;
}

inline f64 Paraboloid(const std::vector<f64>& arg)
{
  f64 val = 0;
  for (usize i = 0; i < arg.size(); i++)
    val += sqr(arg[i] - i - 1);
  return val;
}

inline f64 Rastigrin(const std::vector<f64>& arg)
{
  f64 retval = 0;
  for (const auto& x : arg)
    retval += x * x - 10. * std::cos(2. * Constants::Pi * x);
  return retval;
}

inline f64 Ackley(const std::vector<f64>& arg)
{
  f64 x = arg[0];
  f64 y = arg[1];
  return -20. * exp(-0.2 * sqrt(0.5 * (x * x + y * y))) - exp(0.5 * (cos(2 * Constants::Pi * x) + cos(2 * Constants::Pi * y))) + exp(1.) + 20;
}

inline f64 Himmelblau(const std::vector<f64>& arg)
{
  f64 x = arg[0];
  f64 y = arg[1];
  return sqr(x * x + y - 11) + sqr(x + y * y - 7);
}

inline f64 Rosenbrock(const std::vector<f64>& arg)
{
  f64 x = arg[0];
  f64 y = arg[1];
  static constexpr f64 a = 1;
  static constexpr f64 b = 100;
  return sqr(a - x) + b * sqr(y - sqr(x));
}

inline f64 Beale(const std::vector<f64>& arg)
{
  f64 x = arg[0];
  f64 y = arg[1];
  return sqr(1.5 - x + x * y) + sqr(2.25 - x + x * y * y) + sqr(2.625 - x + x * y * y * y);
}

inline f64 Goldstein(const std::vector<f64>& arg)
{
  f64 x = arg[0];
  f64 y = arg[1];
  return (1 + sqr(x + y + 1) * (19 - 14 * x + 3 * x * x - 14 * y + 6 * x * y + 3 * y * y)) * (30 + sqr(2 * x - 3 * y) * (18 - 32 * x + 12 * x * x + 48 * y - 36 * x * y + 27 * y * y));
}

inline f64 Bukin(const std::vector<f64>& arg)
{
  f64 x = arg[0];
  f64 y = arg[1];
  return 100 * std::sqrt(std::abs(y - 0.01 * x * x)) + 0.01 * std::abs(x + 10);
}

inline f64 Levi(const std::vector<f64>& arg)
{
  f64 x = arg[0];
  f64 y = arg[1];
  f64 pi = Constants::Pi;
  return sqr(std::sin(3. * pi * x)) + sqr(x - 1) * (1 + sqr(std::sin(3 * pi * y))) + sqr(y - 1) * (1 + sqr(std::sin(2 * pi * y)));
}

inline f64 Camel(const std::vector<f64>& arg)
{
  f64 x = arg[0];
  f64 y = arg[1];
  return 2. * x * x - 1.05 * std::pow(x, 4) + std::pow(x, 6) / 6 + x * y + y * y;
}

inline f64 Easom(const std::vector<f64>& arg)
{
  f64 x = arg[0];
  f64 y = arg[1];
  f64 pi = Constants::Pi;
  return -std::cos(x) * std::cos(y) * std::exp(-(sqr(x - pi) + sqr(y - pi)));
}

inline f64 CrossInTray(const std::vector<f64>& arg)
{
  f64 x = arg[0];
  f64 y = arg[1];
  f64 pi = Constants::Pi;
  return -0.0001 * std::pow(std::abs(std::sin(x) * std::sin(y) * std::exp(std::abs(100 - std::sqrt(x * x + y * y) / pi))) + 1, 0.1);
}

inline f64 HolderTable(const std::vector<f64>& arg)
{
  f64 x = arg[0];
  f64 y = arg[1];
  return -std::abs(std::sin(x) * std::cos(y) * std::exp(std::abs(1 - std::sqrt(x * x + y * y) / Constants::Pi)));
}

inline f64 McCormick(const std::vector<f64>& arg)
{
  f64 x = arg[0];
  f64 y = arg[1];
  return std::sin(x + y) + sqr(x - y) - 1.5 * x + 2.5 * y + 1;
}

inline f64 Schaffer(const std::vector<f64>& arg)
{
  f64 x = arg[0];
  f64 y = arg[1];
  return 0.5 + (sqr(std::sin(x * x - y * y)) - 0.5) / sqr(1 + 0.001 * (x * x + y * y));
}

inline f64 Styblinski(const std::vector<f64>& arg)
{
  f64 retval = 0;
  for (const auto& x : arg)
    retval += (std::pow(x, 4) - 16 * x * x + 5 * x) / 2;
  return retval;
}

inline f64 Exp(const std::vector<f64>& arg)
{
  f64 retval = 0;
  for (usize i = 0; i < arg.size(); i++)
    retval += sqr((i + 1) * arg[i] - 1 - i);
  return std::exp(std::sqrt(retval));
}

inline f64 Noisy(const std::vector<f64>& arg)
{
  f64 retval = 0;
  for (usize i = 0; i < arg.size(); i++)
    retval += arg[i] * (std::sin(3 * arg[i]) + 0.3 * rand11());
  return retval / arg.size();
}
}
