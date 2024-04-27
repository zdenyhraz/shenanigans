#pragma once

template <typename... Args>
std::string CreateMessage(std::string_view fmt, Args&&... args)
{
  return fmt::vformat(fmt, fmt::make_format_args(args...));
}

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
  inline static std::recursive_mutex mMutex;
  inline static LogLevel mLogLevel = LogLevel::Trace;
  inline static f32 mProgress = 0;
  inline static std::string mProgressName;
  inline static std::string mProgressBarOverlay;

  template <typename T>
  class Singleton
  {
  public:
    static T& Get()
    {
      static T instance;
      return instance;
    }
  };

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

  inline static void Mute() { mLogLevel = LogLevel::LogLevelCount; }

  inline static std::string GetCurrentDate() { return fmt::format("{:%Y-%b-%d}", fmt::localtime(std::time(nullptr))); }

  inline static std::string GetCurrentTime() { return fmt::format("{:%H:%M:%S}", fmt::localtime(std::time(nullptr))); }
};
