#pragma once
#include "UtilsCV/Vectmat.h"

template <typename I, typename O>
inline void EstimateComplexity(const std::function<O(const std::vector<I>&)>& f, usize nMin = 1000, usize nMax = 10000, usize nIters = 21, usize timeIters = 3)
{
  using clock = std::chrono::high_resolution_clock;
  using time_unit = std::chrono::nanoseconds;

  const usize nStep = (nMax - nMin) / (nIters - 1);
  std::vector<f64> ns(nIters);
  std::vector<f64> times(nIters);
  std::vector<f64> timesZero(nIters);
  cv::Mat Y = cv::Mat::zeros(nIters, 1, CV_64F);
  cv::Mat X = cv::Mat::zeros(nIters, 2, CV_64F);

  const std::vector<I> emptyVec;
  std::ignore = f(emptyVec); // hot cache
  const auto constantStart = clock::now();
  for (usize i = 0; i < timeIters; ++i)
    std::ignore = f(emptyVec);
  const auto constantDuration = static_cast<f64>(std::chrono::duration_cast<time_unit>(clock::now() - constantStart).count()) / timeIters;

#pragma omp parallel for
  for (usize i = 0; i < nIters; i++)
  {
    const i32 n = nMin + i * nStep;
    LOG_DEBUG("Measuring function complexity... (n={})", n);

    std::vector<I> input(n);
    for (auto& in : input)
      in = RandU();

    const auto start = clock::now();
    for (usize ti = 0; ti < timeIters; ++ti)
      std::ignore = f(input);
    const auto duration = static_cast<f64>(std::chrono::duration_cast<time_unit>(clock::now() - start).count()) / timeIters;

    ns[i] = n;
    times[i] = duration;
    timesZero[i] = std::max(duration - constantDuration, 1.0);
    Y.at<f64>(i, 0) = std::log(timesZero[i]);
    X.at<f64>(i, 0) = 1;
    X.at<f64>(i, 1) = std::log(ns[i]);
  }

  cv::Mat coeffs = LeastSquares(Y, X);
  cv::Mat fit = X * coeffs;

  Plot1D::Set("complexity log");
  Plot1D::SetXlabel("log(n)");
  Plot1D::SetYlabel("log(time)");
  Plot1D::SetYnames({"log(timesZero)", "log(timesZero) fit"});
  Plot1D::SetLegendPosition(Plot1D::LegendPosition::BotRight);
  Plot1D::Plot("complexity log", GetCol(X, 1), {GetCol(Y, 0), GetCol(fit, 0)});

  const f64 estimatedScalar = std::exp(coeffs.at<f64>(0, 0));
  const f64 estimatedExponent = coeffs.at<f64>(1, 0);
  LOG_INFO(fmt::format("Estimated complexity: O({:.1f}n^{:.1f})", estimatedScalar, estimatedExponent));

  std::vector<f64> timesFit(fit.rows);
  for (i32 i = 0; i < fit.rows; ++i)
    timesFit[i] = std::exp(fit.at<f64>(i, 0));

  Plot1D::Set("complexity");
  Plot1D::SetXlabel("n");
  Plot1D::SetYlabel("time");
  Plot1D::SetYnames({"times", "timesZero", "timesZero fit"});
  Plot1D::SetYmin(0);
  Plot1D::SetLegendPosition(Plot1D::LegendPosition::BotRight);
  Plot1D::Plot("complexity", ns, {times, timesZero, timesFit});
}
