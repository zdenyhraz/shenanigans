#pragma once
#include <chrono>
#include <string>
#include <fmt/format.h>
#include "Logger.h"

#define LOG_FUNCTION(fun) LogFunction logFunction(fun)

class LogFunction
{
  using clock = std::chrono::high_resolution_clock;
  using time_point = std::chrono::time_point<clock>;

public:
  LogFunction(const std::string& funName) : mFunName(funName), mStartTime(clock::now()) { Logger::Function(fmt::format("{} started", mFunName)); }

  ~LogFunction() { Logger::Function(fmt::format("{} finished ({})", mFunName, FormatDuration(clock::now() - mStartTime))); }

private:
  static std::string FormatDuration(std::chrono::nanoseconds dur)
  {
    using namespace std::chrono;
    using namespace std::chrono_literals;

    if (dur < 1s)
      return fmt::format("{:.2f} ms", duration<f32, std::milli>(dur).count());
    else if (dur < 1min)
      return fmt::format("{:.2f} s", duration<f32>(dur).count());
    else if (dur < 1h)
      return fmt::format("{:.2f} min", duration<f32, std::ratio<60>>(dur).count());
    else
      return fmt::format("{:.2f} h", duration<f32, std::ratio<3600>>(dur).count());
  }

  std::string mFunName;
  time_point mStartTime;
};
