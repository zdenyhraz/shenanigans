#pragma once
#include "stdafx.h"
#include "Utils/vectmat.h"

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

  const size_t nMin = 1000;
  const size_t nMax = 5000;
  const size_t nIters = 101;
  const size_t nStep = (nMax - nMin) / (nIters - 1);
  const size_t timeIters = 3;

  std::vector<double> ns(nIters);
  std::vector<double> times(nIters);
  std::vector<double> timesZero(nIters);
  Mat Y = Mat::zeros(nIters, 1, CV_64F);
  Mat X = Mat::zeros(nIters, 2, CV_64F);

  const std::vector<double> emptyVec;
  std::ignore = f(emptyVec); // hot cache
  const auto constantStart = clock::now();
  for (int i = 0; i < timeIters; ++i)
    std::ignore = f(emptyVec);
  const auto constantDuration = static_cast<double>(std::chrono::duration_cast<time_unit>(clock::now() - constantStart).count()) / timeIters;

#pragma omp parallel for
  for (int i = 0; i < nIters; i++)
  {
    const int n = nMin + i * nStep;
    LOG_DEBUG("Measuring function complexity... (n={})", n);

    std::vector<double> input(n);
    for (auto& in : input)
      in = rand01();

    const auto start = clock::now();
    for (int i = 0; i < timeIters; ++i)
      std::ignore = f(input);
    const auto duration = static_cast<double>(std::chrono::duration_cast<time_unit>(clock::now() - start).count()) / timeIters;

    ns[i] = n;
    times[i] = duration;
    timesZero[i] = std::max(duration - constantDuration, 1.0);
    Y.at<double>(i, 0) = std::log(timesZero[i]);
    X.at<double>(i, 0) = 1;
    X.at<double>(i, 1) = std::log(ns[i]);
  }

  Mat coeffs = LeastSquares(Y, X);
  Mat fit = X * coeffs;

  Plot1D::Set("complexity log");
  Plot1D::SetXlabel("log(n)");
  Plot1D::SetYlabel("log(time)");
  Plot1D::SetYnames({"log(timesZero)", "log(timesZero) fit"});
  Plot1D::Plot("complexity log", GetCol(X, 1), {GetCol(Y, 0), GetCol(fit, 0)});

  const double estimatedScalar = std::exp(coeffs.at<double>(0, 0));
  const double estimatedExponent = coeffs.at<double>(1, 0);
  LOG_INFO(fmt::format("Estimated complexity: O({:.1f}n^{:.1f})", estimatedScalar, estimatedExponent));

  std::vector<double> timesFit(fit.rows);
  for (int i = 0; i < fit.rows; ++i)
    timesFit[i] = std::exp(fit.at<double>(i, 0));

  Plot1D::Set("complexity");
  Plot1D::SetXlabel("n");
  Plot1D::SetYlabel("time");
  Plot1D::SetYnames({"times", "timesZero", "timesZero fit"});
  Plot1D::SetYmin(0);
  Plot1D::Plot("complexity", ns, {times, timesZero, timesFit});
}
