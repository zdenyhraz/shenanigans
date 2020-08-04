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
		spdlog::set_pattern( "[%T] %^%v%$" );
		spdlog::set_level( spdlog::level::debug );
	}

	inline static std::shared_ptr<spdlog::logger> &GetLogger() { return spdlogger; }

private:
	static std::shared_ptr<spdlog::logger> spdlogger;
	//static std::unique_ptr<Logger> logger;
};

class LOG_STARTEND
{
public:
	LOG_STARTEND( std::string &&startmsg, std::string &&endmsg )
	{
		Endmsg = std::move( endmsg );
		LOG_INFO( startmsg );
	}

	~LOG_STARTEND()
	{
		LOG_SUCC( Endmsg );
	}

private:
	std::string Endmsg;
};

bool CheckIfFileExists( const std::string &path );
