#include "Stdafx.h"
#include "Logger.h"

std::shared_ptr<spdlog::logger> Logger::spdlogger;
//std::unique_ptr<Logger> Logger::logger = std::make_unique<Logger>();

bool CheckIfFileExists( const std::string &path )
{
	if ( !std::filesystem::exists( path ) )
	{
		LOG_ERROR( "File {} does not exist", path );
		return false;
	}
	return true;
}
