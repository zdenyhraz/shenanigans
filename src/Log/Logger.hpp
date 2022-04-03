#pragma once

class Logger
{
public:
  enum class LogLevel
  {
    Trace,
    Function,
    Debug,
    Info,
    Success,
    Warning,
    Error,
    LogLevelCount
  };

  static void SetLogLevel(LogLevel logLevel)
  {
    std::scoped_lock lock(mMutex);
    mLogLevel = logLevel;
  }

protected:
  inline static LogLevel mLogLevel = LogLevel::Trace;
  inline static std::mutex mMutex;
  static constexpr usize mMaxMessages = 5000;

  static bool ShouldLog(LogLevel logLevel) { return logLevel >= mLogLevel; }

  static std::string GetCurrentTime()
  {
    auto now = std::time(nullptr);
    char buf[sizeof("12:34:56")];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&now));
    return buf;
  }
};
