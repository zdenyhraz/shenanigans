#pragma once
#include "stdafx.h"

Mat LeastSquares(const Mat& Y, const Mat& X)
{
  if (Y.rows != X.rows)
    throw std::runtime_error("Data count mismatch");

  return (X.t() * X).inv() * X.t() * Y;
}

void EstimateComplexity(const std::function<double(const std::vector<double>)>& f)
{
  using clock = std::chrono::high_resolution_clock;
  using time_unit = std::chrono::nanoseconds;

  const size_t nMin = 10;
  const size_t nMax = 1000;
  const size_t nIters = 201;
  const size_t nStep = (nMax - nMin) / (nIters - 1);

  std::vector<double> ns(nIters);
  std::vector<double> times(nIters);
  Mat Y = Mat::zeros(nIters, 1, CV_64F);
  Mat X = Mat::zeros(nIters, 2, CV_64F);

  for (int n = nMin, i = 0; n <= nMax && i < nIters; n += nStep, i++)
  {
    std::vector<double> input(n);
    for (auto& in : input)
      in = rand01();

    const auto start = clock::now();
    std::ignore = f(input);
    const auto duration = std::chrono::duration_cast<time_unit>(clock::now() - start).count();

    ns[i] = n;
    times[i] = (duration);
    Y.at<double>(i, 0) = std::log(duration);
    X.at<double>(i, 0) = 1;
    X.at<double>(i, 1) = std::log(n);
  }

  Mat coeffs = LeastSquares(Y, X);
  double estimatedScalar = std::exp(coeffs.at<double>(0, 0));
  double estimatedExponent = coeffs.at<double>(1, 0);
  LOG_DEBUG(fmt::format("Estimated scalar: {}", estimatedScalar));
  LOG_DEBUG(fmt::format("Estimated exponent: {}", estimatedExponent));
  LOG_INFO(fmt::format("Estimated complexity: O(n^{:.1f})", estimatedExponent));

  Mat fit = X * coeffs;
  std::vector<double> timesFit(fit.rows);
  for (int i = 0; i < fit.rows; ++i)
    timesFit[i] = std::exp(fit.at<double>(i, 0));

  Plot1D::Reset("complexity");
  Plot1D::SetXlabel("n");
  Plot1D::SetYlabel("time");
  Plot1D::SetYnames({"times", "timesFit"});
  Plot1D::Plot("complexity", ns, {times, timesFit});
}
