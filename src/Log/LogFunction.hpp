#pragma once
#include "Logger.hpp"

template <class MainLogger, bool Show = true>
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
      MainLogger::Message(Logger::LogLevel::Trace, "> {}", mFunName);
    }
  }

  explicit LogFunction(std::string&& funName) : LogFunction(funName.c_str()) {}

  ~LogFunction()
  {
    if constexpr (Show)
      MainLogger::Message(Logger::LogLevel::Trace, "< {} ({})", mFunName, FormatDuration(clock::now() - mStartTime));
  }

private:
  static std::string FormatDuration(std::chrono::nanoseconds dur)
  {
    using namespace std::chrono;

    if (dur < 1s)
      return fmt::format("{:.2f} ms", duration<float, std::milli>(dur).count());
    else if (dur < 1min)
      return fmt::format("{:.2f} s", duration<float>(dur).count());
    else if (dur < 1h)
      return fmt::format("{:.2f} min", duration<float, std::ratio<60>>(dur).count());
    else
      return fmt::format("{:.2f} h", duration<float, std::ratio<3600>>(dur).count());
  }
};
