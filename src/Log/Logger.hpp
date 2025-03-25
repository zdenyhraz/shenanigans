#pragma once

template <typename... Args>
std::string CreateMessage(std::string_view fmt, Args&&... args)
{
  return fmt::vformat(fmt, fmt::make_format_args(args...));
}

class Logger
{
public:
  enum LogLevel
  {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    LogLevelCount
  };

protected:
  inline static std::recursive_mutex mMutex;
  inline static LogLevel mLogLevel = Trace;
  inline static float mProgress = 0;
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

  inline static bool ShouldLog(LogLevel logLevel) { return logLevel >= mLogLevel; }

  static consteval std::array<fmt::rgb, LogLevelCount> GetLogLevelColors()
  {
    std::array<fmt::rgb, LogLevelCount> colors{};
    colors[Trace] = fmt::rgb(172, 136, 255);
    colors[Debug] = fmt::rgb(30, 144, 255);
    colors[Info] = fmt::rgb(87, 226, 143);
    colors[Warning] = fmt::rgb(255, 150, 45);
    colors[Error] = fmt::rgb(255, 55, 55);
    return colors;
  }

public:
  static void SetLogLevel(LogLevel logLevel) { mLogLevel = logLevel; }

  static void SetProgress(float progress)
  {
    mProgress = progress;
    fmt::format_to(mProgressBarOverlay.begin(), "{}% {}", std::clamp(static_cast<int>(mProgress * 100), 0, 100), mProgressName);
  }

  // cppcheck-suppress passedByValue
  static void SetProgressName(std::string_view progressName)
  {
    mProgressName = progressName;
    mProgressBarOverlay.resize(mProgressName.size() + sizeof("100%"));
    fmt::format_to(mProgressBarOverlay.begin(), "{}% {}", std::clamp(static_cast<int>(mProgress * 100), 0, 100), mProgressName);
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
