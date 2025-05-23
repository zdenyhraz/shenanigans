#include <gtest/gtest.h>
#include "Math/TrigonometricFit.hpp"

TEST(TrigfitTest, Fit)
{
  const int n = 101;
  const double c0 = 3.23, c1 = -2.56, c2 = 1.65;
  std::vector<double> x(n), y(n);

  for (int i = 0; i < n; ++i)
  {
    x[i] = static_cast<double>(i) / (n - 1) * 6.28;
    y[i] = c0 + c1 * std::pow(std::sin(x[i]), 2) + c2 * std::pow(std::sin(x[i]), 4);
  }

  const auto fy = TrigonometricFit(x, y);
  ASSERT_EQ(fy.size(), n);
  for (int i = 0; i < n; ++i)
    ASSERT_NEAR(fy[i], y[i], 1e-4);
}

TEST(TrigfitTest, Coeffs)
{
  const int n = 101;
  const double c0 = 3.23, c1 = -2.56, c2 = 1.65;
  std::vector<double> x(n), y(n);

  for (int i = 0; i < n; ++i)
  {
    x[i] = static_cast<double>(i) / (n - 1) * 6.28;
    y[i] = c0 + c1 * std::pow(std::sin(x[i]), 2) + c2 * std::pow(std::sin(x[i]), 4);
  }

  const auto fc = TrigonometricFitCoefficients(x, y);
  ASSERT_EQ(fc.size(), 3);
  ASSERT_NEAR(fc[0], c0, 1e-4);
  ASSERT_NEAR(fc[1], c1, 1e-4);
  ASSERT_NEAR(fc[2], c2, 1e-4);
}
