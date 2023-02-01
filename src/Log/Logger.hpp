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

  static void SetLogLevel(LogLevel logLevel) { mLogLevel = logLevel; }
  static void SetProgress(f32 progress) { mProgress = progress; }
  static void SetProgressName(std::string_view progressName) { mProgressName = progressName; }
  static void ResetProgress()
  {
    mProgress = 0;
    mProgressName.clear();
  }

protected:
  inline static LogLevel mLogLevel = LogLevel::Trace;
  inline static f32 mProgress = 0;
  inline static std::string mProgressName;
  inline static std::mutex mMutex;

  bool ShouldLog(LogLevel logLevel) const { return logLevel >= mLogLevel; }
};
