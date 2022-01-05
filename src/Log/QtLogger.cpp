#include "QtLogger.h"

QtLogger::QtLogger()
{
  mLogLevelSettings[LogLevel::Trace] = {QColor(150, 150, 150), "Trace"};
  mLogLevelSettings[LogLevel::Function] = {QColor(178, 102, 255), "Function"};
  mLogLevelSettings[LogLevel::Debug] = {QColor(51, 153, 255), "Debug"};
  mLogLevelSettings[LogLevel::Info] = {QColor(205, 255, 0), "Info"};
  mLogLevelSettings[LogLevel::Success] = {QColor(0, 204, 0), "Success"};
  mLogLevelSettings[LogLevel::Warning] = {QColor(255, 154, 20), "Warning"};
  mLogLevelSettings[LogLevel::Error] = {QColor(225, 0, 0), "Error"};
}
