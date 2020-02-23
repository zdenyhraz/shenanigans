#pragma once
#include "stdafx.h"

class Logger
{
public:
	Logger()
	{
		logger = spdlog::stdout_color_mt("console");
		spdlog::set_pattern("%^[%T][%l] %v%$");
		spdlog::set_level(spdlog::level::debug);
	}

	inline static std::shared_ptr<spdlog::logger>& GetLogger()
	{
		return logger;
	}

private:
	static std::shared_ptr<spdlog::logger> logger;
};

#define LOG_DEBUG(...) Logger::GetLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...) Logger::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) Logger::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) Logger::GetLogger()->error(__VA_ARGS__)
#define LOG_FATAL(...) Logger::GetLogger()->critical(__VA_ARGS__)