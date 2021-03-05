#pragma once
#include "stdafx.h"
#include "Logger.h"

#define LOG_FUNCTION(fun) LogFunction logFunction(fun)

class LogFunction
{
  using clock = std::chrono::high_resolution_clock;
  using time_point = std::chrono::time_point<clock>;

public:
  LogFunction(const std::string& funName) : mFunName(funName), mStartTime(clock::now()) { LOG_DEBUG(fmt::format("{} started", mFunName)); }

  ~LogFunction()
  {
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - mStartTime).count();
    LOG_DEBUG(fmt::format("{} finished ({})", mFunName, FormatDuration(duration)));
  }

private:
  static std::string FormatDuration(long long durationms)
  {
    static constexpr long long kSecondMs = 1000;
    static constexpr long long kMinuteMs = kSecondMs * 60;
    static constexpr long long kHourMs = kMinuteMs * 60;

    if (durationms < kSecondMs)
      return fmt::format("{} ms", durationms);
    else if (durationms < kMinuteMs)
      return fmt::format("{:.2f} s", (double)durationms / kSecondMs);
    else if (durationms < kHourMs)
      return fmt::format("{:.2f} m", (double)durationms / kMinuteMs);
    else
      return fmt::format("{:.2f} h", (double)durationms / kHourMs);
  }

  std::string mFunName;
  time_point mStartTime;
};
