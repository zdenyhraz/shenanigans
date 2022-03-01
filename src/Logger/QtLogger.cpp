#include "QtLogger.h"

QtLogger::QtLogger()
{
  mLogLevelSettings[LogLevel::Trace] = {.color = QColor(150, 150, 150)};
  mLogLevelSettings[LogLevel::Function] = {.color = QColor(150, 150, 150)}; //{QColor(178, 102, 255)};
  mLogLevelSettings[LogLevel::Debug] = {.color = QColor(51, 153, 255)};
  mLogLevelSettings[LogLevel::Info] = {.color = QColor(2, 190, 230), .italic = true};
  mLogLevelSettings[LogLevel::Success] = {.color = QColor(0, 204, 0), .italic = true};
  mLogLevelSettings[LogLevel::Warning] = {.color = QColor(255, 154, 20)};
  mLogLevelSettings[LogLevel::Error] = {.color = QColor(225, 0, 0)};
}
