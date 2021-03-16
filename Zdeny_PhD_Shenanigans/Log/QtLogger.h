#pragma once
#include "stdafx.h"

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

  static void SetTextBrowser(QTextBrowser* textBrowser) { Get().mTextBrowser = textBrowser; }
  static void SetLogLevel(LogLevel logLevel) { Get().mLogLevel = logLevel; }

  template <typename... Args> static void Trace(const std::string& fmt, Args&&... args) { Get().LogMessage(LogLevel::Trace, fmt, args...); }
  template <typename... Args> static void Function(const std::string& fmt, Args&&... args) { Get().LogMessage(LogLevel::Function, fmt, args...); }
  template <typename... Args> static void Debug(const std::string& fmt, Args&&... args) { Get().LogMessage(LogLevel::Debug, fmt, args...); }
  template <typename... Args> static void Info(const std::string& fmt, Args&&... args) { Get().LogMessage(LogLevel::Info, fmt, args...); }
  template <typename... Args> static void Success(const std::string& fmt, Args&&... args) { Get().LogMessage(LogLevel::Success, fmt, args...); }
  template <typename... Args> static void Warning(const std::string& fmt, Args&&... args) { Get().LogMessage(LogLevel::Warning, fmt, args...); }
  template <typename... Args> static void Error(const std::string& fmt, Args&&... args) { Get().LogMessage(LogLevel::Error, fmt, args...); }

private:
  QtLogger()
  {
    mLogLevelSettings[LogLevel::Trace] = {QColor(150, 150, 150), "Trace"};
    mLogLevelSettings[LogLevel::Function] = {QColor(108, 32, 185), "Function"}; // QColor(178, 102, 255)
    mLogLevelSettings[LogLevel::Debug] = {QColor(51, 153, 255), "Debug"};
    mLogLevelSettings[LogLevel::Info] = {QColor(205, 255, 0), "Info"};
    mLogLevelSettings[LogLevel::Success] = {QColor(0, 204, 0), "Success"};
    mLogLevelSettings[LogLevel::Warning] = {QColor(255, 154, 20), "Warning"};
    mLogLevelSettings[LogLevel::Error] = {QColor(225, 0, 0), "Error"};
  }

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

  bool ShouldLog(LogLevel logLevel) { return mTextBrowser != nullptr && logLevel >= mLogLevel; }

  static std::string GetCurrentTime()
  {
    time_t now = time(0);
    char buf[sizeof "12:34:56"];
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now));
    return buf;
  }

  template <typename... Args> void LogMessage(LogLevel logLevel, const std::string& fmt, Args&&... args)
  {
    std::scoped_lock lock(mMutex);
    if (!ShouldLog(logLevel))
      return;

    const auto& [color, name] = mLogLevelSettings[logLevel];

    mTextBrowser->setTextColor(color);
    mTextBrowser->append(fmt::format("[{}] [{}] {}", GetCurrentTime(), name, fmt::format(fmt, args...)).c_str());
    QCoreApplication::processEvents();
  }

  QTextBrowser* mTextBrowser = nullptr;
  LogLevel mLogLevel = LogLevel::Function;
  std::unordered_map<LogLevel, LogLevelSettings> mLogLevelSettings;
  std::mutex mMutex;
};
