#pragma once
#include "Utils/Filesystem.hpp"

#ifdef _MSC_VER
#  include <stacktrace>
#  define HAS_STACKTRACE 1
#else
#  define HAS_STACKTRACE 0
#endif

class ShenanigansException : public std::exception
{
  std::string mMessage;
#if HAS_STACKTRACE
  std::stacktrace mStacktrace;
#endif

public:
#if HAS_STACKTRACE
  ShenanigansException(const std::string& message, const std::source_location& source, std::stacktrace&& stacktrace = std::stacktrace::current()) :
    mMessage(fmt::format("{} [{}:{}]", message, std::filesystem::relative(std::filesystem::path(source.file_name()), GetProjectDirectoryPath("src")).string(), source.line())),
    mStacktrace(std::move(stacktrace))
  {
  }
#else
  ShenanigansException(const std::string& message, const std::source_location& source) :
    mMessage(fmt::format("{} [{}:{}]", message, std::filesystem::relative(std::filesystem::path(source.file_name()), GetProjectDirectoryPath("src")).string(), source.line()))
  {
  }
#endif

  const char* what() const noexcept override { return mMessage.c_str(); }

  void Log() const
  {
    LOG_ERROR(mMessage);
#if HAS_STACKTRACE
    LOG_WARNING("Exception stack trace:");
    size_t index = 0;
    for (const auto& entry : mStacktrace)
      LOG_WARNING("[{}] {} at {}:{}", index++, entry.description(), entry.source_file(), entry.source_line());
#endif
  }
};

class ShenanigansExceptionPublic : public std::exception
{
  std::string mMessage;

public:
  ShenanigansExceptionPublic(const std::string& message) : mMessage(message) {}

  const char* what() const noexcept override { return mMessage.c_str(); }

  void Log() const { LOG_ERROR(mMessage); }
};
