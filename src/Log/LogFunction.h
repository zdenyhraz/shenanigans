#pragma once

#include "Logger.h"

#define LOG_FUNCTION(fun) LogFunction logFunction(fun)

class LogFunction
{
  using clock = std::chrono::high_resolution_clock;
  using time_point = std::chrono::time_point<clock>;

public:
  LogFunction(const std::string& funName) : mFunName(funName), mStartTime(clock::now()) { Logger::Function(fmt::format("{} started", mFunName)); }

  ~LogFunction()
  {
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - mStartTime).count();
    Logger::Function(fmt::format("{} finished ({})", mFunName, FormatDuration(duration)));
  }

private:
  static std::string FormatDuration(i64 durationms)
  {
    static constexpr i64 kSecondMs = 1000;
    static constexpr i64 kMinuteMs = kSecondMs * 60;
    static constexpr i64 kHourMs = kMinuteMs * 60;

    if (durationms < kSecondMs)
      return fmt::format("{} ms", durationms);
    else if (durationms < kMinuteMs)
      return fmt::format("{:.2f} s", (f64)durationms / kSecondMs);
    else if (durationms < kHourMs)
      return fmt::format("{:.2f} m", (f64)durationms / kMinuteMs);
    else
      return fmt::format("{:.2f} h", (f64)durationms / kHourMs);
  }

  std::string mFunName;
  time_point mStartTime;
};
