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

  static consteval std::array<fmt::rgb, static_cast<int>(LogLevel::LogLevelCount)> GetLogLevelColors()
  {
    std::array<fmt::rgb, static_cast<int>(LogLevel::LogLevelCount)> colors{};
    colors[static_cast<size_t>(LogLevel::Trace)] = fmt::rgb(125, 125, 125);
    colors[static_cast<size_t>(LogLevel::Function)] = fmt::rgb(125, 125, 125);
    colors[static_cast<size_t>(LogLevel::Debug)] = fmt::rgb(0, 185, 255);
    colors[static_cast<size_t>(LogLevel::Info)] = fmt::rgb(235, 235, 235);
    colors[static_cast<size_t>(LogLevel::Success)] = fmt::rgb(0, 200, 15);
    colors[static_cast<size_t>(LogLevel::Warning)] = fmt::rgb(255, 90, 0);
    colors[static_cast<size_t>(LogLevel::Error)] = fmt::rgb(255, 0, 0);
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
