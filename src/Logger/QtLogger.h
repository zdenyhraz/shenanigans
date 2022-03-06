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
  static QtLogger& Get()
  {
    static QtLogger logger;
    return logger;
  }

  struct LogLevelSettings
  {
    QColor color = QColor(255, 255, 255);
    bool italic = false;
  };

  QtLogger();

  bool ShouldLog(LogLevel logLevel) const { return mTextBrowser and logLevel >= mLogLevel; }

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
    auto now = time(nullptr);
    char buf[sizeof "12:34:56"];
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now));
    return buf;
  }

  template <typename... Args>
  void LogMessage(LogLevel logLevel, const std::string& fmt, Args&&... args)
  {
    if (not ShouldLog(logLevel))
      [[unlikely]] return;

    std::scoped_lock lock(mMutex);
    const auto& settings = mLogLevelSettings[logLevel];
    mTextBrowser->setTextColor(settings.color);
    mTextBrowser->setFontItalic(settings.italic);
    mTextBrowser->append(fmt::format("[{}] {}", GetCurrentTime(), fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...))).c_str());
    QCoreApplication::processEvents();
  }

  QTextBrowser* mTextBrowser = nullptr;
  LogLevel mLogLevel = LogLevel::Function;
  std::map<LogLevel, LogLevelSettings> mLogLevelSettings;
  std::mutex mMutex;
};
