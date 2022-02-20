#pragma once

class Profiler
{
  using clock = std::chrono::high_resolution_clock;
  using tp = std::chrono::time_point<clock>;
  using dur = std::chrono::milliseconds;

public:
  struct Data
  {
    usize nmin, nmax, nstep, iters;
    std::vector<std::string> labels;
  };

  template <typename O, typename I>
  static void Profile(std::initializer_list<std::function<O(const I&)>> funcs, const std::function<I(usize)>& getinput, Data&& data)
  {
    using namespace std::chrono;
    std::vector<std::vector<f64>> durations(funcs.size());
    std::vector<f64> ns;
    for (usize fun = 0; fun < funcs.size(); ++fun)
      durations[fun].reserve((data.nmax - data.nmin) / data.nstep);
    ns.reserve((data.nmax - data.nmin) / data.nstep);

    const auto inputwarmup = getinput(data.nmin);
    for (const auto& fun : funcs)
      fun(inputwarmup);

    for (usize n = data.nmin; n <= data.nmax; n += data.nstep)
    {
      LOG_DEBUG("Profiling {} / {} ...", n, data.nmax);
      ns.push_back(n);
      const auto input = getinput(n);
      usize funindex = 0;
      for (const auto& fun : funcs)
      {
        const auto start = clock::now();
        for (usize i = 0; i < data.iters; ++i)
          fun(input);
        const auto end = clock::now();

        durations[funindex++].push_back(static_cast<f64>(duration_cast<microseconds>(end - start).count()) / data.iters);
      }
    }

    PyPlot::Plot("Profiling", {.x = ns, .ys = durations, .xlabel = "n", .ylabel = fmt::format("mean time per call ({}) [us]", data.iters), .label_ys = data.labels});
  }
};