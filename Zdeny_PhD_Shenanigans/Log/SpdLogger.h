#pragma once
#include "stdafx.h"

class SpdLogger
{
public:
  static spdlog::logger& Get()
  {
    static SpdLogger logger;
    return *logger.mLogger;
  }

private:
  SpdLogger()
  {
    mLogger = spdlog::stdout_color_mt("console");
    mLogger->set_pattern("[%T] %^%v%$");
    mLogger->set_level(spdlog::level::trace);
  }

  std::shared_ptr<spdlog::logger> mLogger;
};
