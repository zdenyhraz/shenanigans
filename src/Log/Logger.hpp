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

  void SetLogLevel(LogLevel logLevel)
  {
    std::scoped_lock lock(mMutex);
    mLogLevel = logLevel;
  }

protected:
  LogLevel mLogLevel = LogLevel::Trace;
  std::mutex mMutex;

  bool ShouldLog(LogLevel logLevel) { return logLevel >= mLogLevel; }
};
