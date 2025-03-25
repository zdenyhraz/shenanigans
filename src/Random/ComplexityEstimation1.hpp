#pragma once

class Profiler
{
public:
  struct Data
  {
    size_t nmin = 1, nmax = 1000, nstep = 1, iters = 100;
    std::vector<std::string> labels = {"fun 1", "fun 2"};
  };

  template <typename O, typename I>
  static void Profile(std::initializer_list<std::function<O(const I&)>> funcs, const std::function<I(size_t)>& getinput, Data&& data)
  {
    using namespace std::chrono;
    const size_t nsize = (data.nmax - data.nmin) / data.nstep + 1;
    std::vector<std::vector<double>> durations = Zerovect2(funcs.size(), nsize, 0.);
    std::vector<std::vector<double>> durationfits = Zerovect2(funcs.size(), nsize, 0.);
    std::vector<double> ns(nsize);
    std::vector<cv::Mat> Y(funcs.size());
    for (auto& y : Y)
      y = cv::Mat::zeros(nsize, 1, CV_64F);
    cv::Mat X = cv::Mat::zeros(nsize, 2, CV_64F);
    std::vector<double> complexityMultipliers(funcs.size(), 0.);
    std::vector<double> complexityExponents(funcs.size(), 0.);

    const auto inputwarmup = getinput(data.nmin);
    for (const auto& fun : funcs)
      fun(inputwarmup);

    for (size_t nindex = 0; nindex < nsize; ++nindex)
    {
      size_t n = data.nmin + nindex * data.nstep;
      LOG_DEBUG("Profiling {} / {}", n, data.nmax);
      ns[nindex] = n;
      X.at<double>(nindex, 0) = 1;
      X.at<double>(nindex, 1) = std::log(n);
      const auto input = getinput(n);
      size_t funindex = 0;
      for (const auto& fun : funcs)
      {
        const auto start = high_resolution_clock::now();
        for (size_t i = 0; i < data.iters; ++i)
          fun(input);
        const auto end = high_resolution_clock::now();

        const auto dur = static_cast<double>(duration_cast<microseconds>(end - start).count()) / data.iters;
        durations[funindex][nindex] = dur;
        Y[funindex].at<double>(nindex, 0) = std::log(dur);
        ++funindex;
      }
    }

    std::vector<std::string> label_ys(funcs.size());
    std::vector<std::string> label_y2s(funcs.size());
    for (size_t funindex = 0; funindex < funcs.size(); ++funindex)
    {
      const cv::Mat coeffs = LeastSquares(Y[funindex], X);
      complexityMultipliers[funindex] = std::exp(coeffs.at<double>(0, 0));
      complexityExponents[funindex] = coeffs.at<double>(1, 0);
      label_ys[funindex] = data.labels.size() > funindex ? data.labels[funindex] : fmt::format("fun {}", funindex);
      label_y2s[funindex] = fmt::format("{} fit - O({:.1g}n^{:.1f})", label_ys[funindex], complexityMultipliers[funindex], complexityExponents[funindex]);
      const cv::Mat fit = X * coeffs;
      for (size_t nindex = 0; nindex < nsize; ++nindex)
        durationfits[funindex][nindex] = std::exp(fit.at<double>(nindex, 0));
      LOG_INFO("Function {} estimated complexity: O({:.1g}n^{:.1f})", label_ys[funindex], complexityMultipliers[funindex], complexityExponents[funindex]);
    }

    // append fit data and labels to the end
    durations.insert(durations.end(), durationfits.begin(), durationfits.end());
    label_ys.insert(label_ys.end(), label_y2s.begin(), label_y2s.end());
    Plot::Plot({.name = "Profiling", .x = ns, .ys = durations, .xlabel = "n", .ylabel = fmt::format("mean time per call ({}) [us]", data.iters), .ylabels = label_ys});
  }

  template <typename O, typename I>
  static void Profile(const std::function<O(const I&)>& func1, const std::function<O(const I&)>& func2, const std::function<I(size_t)>& getinput, Data&& data)
  {
    return Profile({func1, func2}, getinput, std::move(data));
  }

  template <typename O, typename I>
  static void Profile(const std::function<O(const I&)>& func, const std::function<I(size_t)>& getinput, Data&& data)
  {
    return Profile(func, getinput, std::move(data));
  }
};
