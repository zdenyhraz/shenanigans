#pragma once
#define TIMER(name) Timer timer(name);

class Timer
{
public:
  using clock = std::chrono::high_resolution_clock;
  using tp = std::chrono::time_point<clock>;
  using dur = std::chrono::milliseconds;

  std::string name;
  tp stp;

  static constexpr auto tse(const tp& tmp) { return std::chrono::time_point_cast<dur>(tmp).time_since_epoch().count(); }

  explicit Timer(const std::string& name) : name(name), stp(clock::now()) {}
  ~Timer() { LOG_INFO("{} took {} ms", name, tse(clock::now()) - tse(stp)); }
};
