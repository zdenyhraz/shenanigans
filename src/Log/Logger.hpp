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

protected:
  inline static std::mutex mMutex;
  inline static LogLevel mLogLevel = LogLevel::Trace;
  inline static f32 mProgress = 0;
  inline static std::string mProgressName;
  inline static std::string mProgressBarOverlay;

  bool ShouldLog(LogLevel logLevel) const { return logLevel >= mLogLevel; }

public:
  static void SetLogLevel(LogLevel logLevel) { mLogLevel = logLevel; }

  static void SetProgress(f32 progress)
  {
    mProgress = progress;
    fmt::format_to(mProgressBarOverlay.begin(), "{}% {}", std::clamp(static_cast<i32>(mProgress * 100), 0, 100), mProgressName);
  }

  static void SetProgressName(std::string_view progressName)
  {
    mProgressName = progressName;
    mProgressBarOverlay.resize(mProgressName.size() + sizeof("100%"));
    fmt::format_to(mProgressBarOverlay.begin(), "{}% {}", std::clamp(static_cast<i32>(mProgress * 100), 0, 100), mProgressName);
  }

  static void ResetProgress()
  {
    mProgress = 0;
    mProgressName.clear();
    mProgressBarOverlay.clear();
  }
};
