#pragma once
#include "SpdLogger.h"

#define LOG_TRACE(...) SpdLogger::Get()->trace(__VA_ARGS__)
#define LOG_DEBUG(...) SpdLogger::Get()->debug(__VA_ARGS__)
#define LOG_SUCC(...) SpdLogger::Get()->info(__VA_ARGS__)
#define LOG_INFO(...) SpdLogger::Get()->warn(__VA_ARGS__)
#define LOG_ERROR(...) SpdLogger::Get()->error(__VA_ARGS__)
#define LOG_FATAL(...) SpdLogger::Get()->critical(__VA_ARGS__)

#define LOG_NEWLINE SpdLogger::Get()->debug("")
#define LOG_FUNCTION(fun) LOG_FUNCTION_IMPL log_function_impl(fun)

class LOG_FUNCTION_IMPL
{
  using clock = std::chrono::high_resolution_clock;
  using time_point = std::chrono::time_point<clock>;
  using duration = std::chrono::milliseconds;

public:
  LOG_FUNCTION_IMPL(const std::string& funName) : mFunName(funName), mStartTime(clock::now()) { LOG_DEBUG(fmt::format("{} started", mFunName)); }

  ~LOG_FUNCTION_IMPL()
  {
    const auto duration = TimeSinceEpoch(clock::now()) - TimeSinceEpoch(mStartTime);
    LOG_DEBUG(fmt::format("{} finished ({})", mFunName, FormatDuration(duration)));
  }

private:
  static constexpr long long TimeSinceEpoch(const time_point& tmp) { return std::chrono::time_point_cast<duration>(tmp).time_since_epoch().count(); }

  static std::string FormatDuration(long long durationms)
  {
    static constexpr long long kSecondMs = 1000;
    static constexpr long long kMinuteMs = kSecondMs * 60;
    static constexpr long long kHourMs = kMinuteMs * 60;

    if (durationms < kSecondMs)
      return fmt::format("{} ms", durationms);
    else if (durationms < kMinuteMs)
      return fmt::format("{} s", (double)durationms / kSecondMs);
    else if (durationms < kHourMs)
      return fmt::format("{} m", (double)durationms / kMinuteMs);
    else
      return fmt::format("{} h", (double)durationms / kHourMs);
  }

  std::string mFunName;
  time_point mStartTime;
};
