#pragma once
#include "Log.hpp"

#define LOG_FUNCTION(funName) LogFunction logFunction(funName)
#define LOG_FUNCTION_IF(show, funName) LogFunction<show> logFunction(funName)

template <bool Show = true>
class LogFunction
{
  using clock = std::chrono::high_resolution_clock;
  using time_point = std::chrono::time_point<clock>;

public:
  explicit LogFunction(std::string&& funName)
  try
  {
    if constexpr (Show)
    {
      mStartTime = clock::now();
      mFunName = std::move(funName);
      Log::Function("{} started", mFunName);
    }
  }
  catch (...)
  {
  }

  ~LogFunction()
  try
  {
    if constexpr (Show)
      Log::Function("{} finished ({})", mFunName, FormatDuration(clock::now() - mStartTime));
  }
  catch (...)
  {
  }

private:
  static std::string FormatDuration(std::chrono::nanoseconds dur)
  {
    using namespace std::chrono;

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
