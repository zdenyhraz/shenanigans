#pragma once

#define LOG_DEBUG(...) Logger::GetLogger()->debug(__VA_ARGS__)
#define LOG_SUCC(...) Logger::GetLogger()->info(__VA_ARGS__)
#define LOG_INFO(...) Logger::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) Logger::GetLogger()->error(__VA_ARGS__)
#define LOG_FATAL(...) Logger::GetLogger()->critical(__VA_ARGS__)
#define LOG_NEWLINE Logger::GetLogger()->debug("")

class Logger
{
public:
	Logger()
	{
		spdlogger = spdlog::stdout_color_mt( "console" );
		spdlog::set_pattern( "%^[%T]%$ %v " );
		spdlog::set_level( spdlog::level::debug );
	}

	inline static std::shared_ptr<spdlog::logger> &GetLogger() { return spdlogger; }

private:
	static std::shared_ptr<spdlog::logger> spdlogger;
};

