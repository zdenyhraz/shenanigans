#include "Stdafx.h"
#include "Logger.h"

std::shared_ptr<spdlog::logger> Logger::spdlogger;
std::unique_ptr<Logger> Logger::logger;
