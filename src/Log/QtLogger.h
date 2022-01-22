#pragma once

class QtLogger
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

  static void SetTextBrowser(QTextBrowser* textBrowser) { Get().SetTextBrowserInternal(textBrowser); }
  static void SetLogLevel(LogLevel logLevel) { Get().SetLogLevelInternal(logLevel); }

  template <typename... Args>
  static void Trace(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Trace, fmt, args...);
  }
  template <typename... Args>
  static void Function(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Function, fmt, args...);
  }
  template <typename... Args>
  static void Debug(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Debug, fmt, args...);
  }
  template <typename... Args>
  static void Info(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Info, fmt, args...);
  }
  template <typename... Args>
  static void Success(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Success, fmt, args...);
  }
  template <typename... Args>
  static void Warning(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Warning, fmt, args...);
  }
  template <typename... Args>
  static void Error(const std::string& fmt, Args&&... args)
  {
    Get().LogMessage(LogLevel::Error, fmt, args...);
  }

private:
  QtLogger();

  struct LogLevelSettings
  {
    QColor color;
    std::string name;
  };

  static QtLogger& Get()
  {
    static QtLogger logger;
    return logger;
  }

  bool ShouldLog(LogLevel logLevel) { return mTextBrowser and logLevel >= mLogLevel; }

  void SetTextBrowserInternal(QTextBrowser* textBrowser)
  {
    std::scoped_lock lock(mMutex);
    mTextBrowser = textBrowser;
  }

  void SetLogLevelInternal(LogLevel logLevel)
  {
    std::scoped_lock lock(mMutex);
    mLogLevel = logLevel;
  }

  static std::string GetCurrentTime()
  {
    time_t now = time(0);
    char buf[sizeof "12:34:56"];
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now));
    return buf;
  }

  template <typename... Args>
  void LogMessage(LogLevel logLevel, const std::string& fmt, Args&&... args)
  {
    if (not ShouldLog(logLevel)) [[unlikely]]
      return;

    std::scoped_lock lock(mMutex);
    const auto& [color, name] = mLogLevelSettings[logLevel];
    mTextBrowser->setTextColor(color);
    mTextBrowser->append(fmt::format("[{}] {}", GetCurrentTime(), fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...))).c_str());
    QCoreApplication::processEvents();
  }

  QTextBrowser* mTextBrowser = nullptr;
  LogLevel mLogLevel = LogLevel::Function;
  std::unordered_map<LogLevel, LogLevelSettings> mLogLevelSettings;
  std::mutex mMutex;
};
