#pragma once
#define LOG_DEBUG(...) Logger::Get()->debug(__VA_ARGS__)
#define LOG_SUCC(...) Logger::Get()->info(__VA_ARGS__)
#define LOG_INFO(...) Logger::Get()->warn(__VA_ARGS__)
#define LOG_ERROR(...) Logger::Get()->error(__VA_ARGS__)
#define LOG_FATAL(...) Logger::Get()->critical(__VA_ARGS__)
#define LOG_NEWLINE Logger::Get()->debug("")
#define LOG_DEBUG_IF(c, ...) if (c) Logger::Get()->debug(__VA_ARGS__)
#define LOG_SUCC_IF(c, ...) if (c) Logger::Get()->info(__VA_ARGS__)
#define LOG_INFO_IF(c, ...) if (c) Logger::Get()->warn(__VA_ARGS__)
#define LOG_ERROR_IF(c, ...) if (c) Logger::Get()->error(__VA_ARGS__)
#define LOG_FATAL_IF(c, ...) if (c) Logger::Get()->critical(__VA_ARGS__)
#define LOG_NEWLINE_IF(c) if (c) Logger::Get()->debug("")
#define LOG_STARTEND(s,e) std::unique_ptr<LOG_STARTEND_IMPL> log_startend_impl = std::make_unique<LOG_STARTEND_IMPL>(s,e)

class Logger
{
public:
	Logger()
	{
		spdlogger = spdlog::stdout_color_mt( "console" );
		spdlog::set_pattern( "[%T] %^%v%$" );
		spdlog::set_level( spdlog::level::debug );
	}

	inline static std::shared_ptr<spdlog::logger> &Get() { return spdlogger; }
	inline static void Init() { logger = std::make_unique<Logger>(); }

private:
	static std::shared_ptr<spdlog::logger> spdlogger;
	static std::unique_ptr<Logger> logger;
};

class LOG_STARTEND_IMPL
{
public:
	LOG_STARTEND_IMPL( const std::string &startMsg, const std::string &endMsg )
	{
		EndMsg = endMsg;
		LOG_INFO( startMsg );
	}

	~LOG_STARTEND_IMPL()
	{
		LOG_SUCC( EndMsg );
	}

private:
	std::string EndMsg;
};


