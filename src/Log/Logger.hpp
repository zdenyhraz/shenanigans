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
    Error
  };

  void SetLogLevel(LogLevel logLevel) { mLogLevel = logLevel; }

protected:
  LogLevel mLogLevel = LogLevel::Function;
};
