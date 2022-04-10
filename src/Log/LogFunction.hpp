#pragma once
#include "Log.hpp"

#define LOG_FUNCTION LogFunction logFunction(std::source_location::current().function_name())
#define LOG_FUNCTION_IF(show) LogFunction<show> logFunction(std::source_location::current().function_name())

#define LOG_SCOPE(funName) LogFunction logScope(funName)
#define LOG_SCOPE_IF(show, funName) LogFunction<show> logScope(funName)

template <bool Show = true>
class LogFunction
{
  using clock = std::chrono::high_resolution_clock;
  using time_point = std::chrono::time_point<clock>;

  std::string mFunName;
  time_point mStartTime;

public:
  explicit LogFunction(const char* funName)
  {
    if constexpr (Show)
    {
      mStartTime = clock::now();
      mFunName = funName;
      MainLogger::Get().Message(Logger::LogLevel::Function, "{} started", mFunName);
    }
  }

  explicit LogFunction(std::string&& funName) : LogFunction(funName.c_str()) {}

  ~LogFunction()
  {
    if constexpr (Show)
      MainLogger::Get().Message(Logger::LogLevel::Function, "{} finished ({})", mFunName, FormatDuration(clock::now() - mStartTime));
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
};
