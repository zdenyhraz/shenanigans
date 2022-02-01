#include "QtLogger.h"

QtLogger::QtLogger()
{
  mLogLevelSettings[LogLevel::Trace] = {QColor(150, 150, 150), false};
  mLogLevelSettings[LogLevel::Function] = {QColor(178, 102, 255), false};
  mLogLevelSettings[LogLevel::Debug] = {QColor(51, 153, 255), false};
  mLogLevelSettings[LogLevel::Info] = {QColor(2, 190, 230), true};
  mLogLevelSettings[LogLevel::Success] = {QColor(0, 204, 0), true};
  mLogLevelSettings[LogLevel::Warning] = {QColor(255, 154, 20), false};
  mLogLevelSettings[LogLevel::Error] = {QColor(225, 0, 0), false};
}
