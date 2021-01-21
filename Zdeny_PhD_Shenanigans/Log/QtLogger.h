#pragma once
#include "stdafx.h"

class QtLogger
{
  enum class LogLevel
  {
    Trace,
    Debug,
    Info,
    Success,
    Warning,
    Error
  };

public:
  static void SetTextBrowser(QTextBrowser* textBrowser) { Get().SetTextBrowserInternal(textBrowser); }

  template <typename... Args> static void Trace(const std::string& fmt, Args&&... args) { Get().LogMessage(LogLevel::Trace, fmt, args...); }
  template <typename... Args> static void Debug(const std::string& fmt, Args&&... args) { Get().LogMessage(LogLevel::Debug, fmt, args...); }
  template <typename... Args> static void Info(const std::string& fmt, Args&&... args) { Get().LogMessage(LogLevel::Info, fmt, args...); }
  template <typename... Args> static void Success(const std::string& fmt, Args&&... args) { Get().LogMessage(LogLevel::Success, fmt, args...); }
  template <typename... Args> static void Warning(const std::string& fmt, Args&&... args) { Get().LogMessage(LogLevel::Warning, fmt, args...); }
  template <typename... Args> static void Error(const std::string& fmt, Args&&... args) { Get().LogMessage(LogLevel::Error, fmt, args...); }

private:
  QtLogger()
  {
    mColors[LogLevel::Trace] = QColor(180, 180, 180);
    mColors[LogLevel::Debug] = QColor(51, 153, 255);
    mColors[LogLevel::Info] = QColor(179, 91, 255);
    mColors[LogLevel::Success] = QColor(0, 209, 56);
    mColors[LogLevel::Warning] = QColor(243, 154, 20);
    mColors[LogLevel::Error] = QColor(225, 0, 0);
  }

  static QtLogger& Get()
  {
    static QtLogger logger;
    return logger;
  }

  bool CanLog() { return mTextBrowser != nullptr; }
  std::string GetTime() { return "11:23:05"; }
  void SetTextBrowserInternal(QTextBrowser* textBrowser) { mTextBrowser = textBrowser; }

  template <typename... Args> void LogMessage(LogLevel logLevel, const std::string& fmt, Args&&... args)
  {
    if (!CanLog())
      return;

    mTextBrowser->setTextColor(mColors[logLevel]);
    mTextBrowser->append(fmt::format("[{}] {}", GetTime(), fmt::format(fmt, args...)).c_str());
    QCoreApplication::processEvents();
  }

  QTextBrowser* mTextBrowser = nullptr;

  std::unordered_map<LogLevel, QColor> mColors;
};
