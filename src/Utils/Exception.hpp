#pragma once
#include "Utils/Filesystem.hpp"

#ifdef _MSC_VER
#  include <stacktrace>
#  define HAS_STACKTRACE 1
#endif

class ShenanigansException : public std::exception
{
  std::string mMessage;
#ifdef HAS_STACKTRACE
  std::stacktrace mStacktrace;
#endif

public:
#ifdef DEVELOP
#  ifdef HAS_STACKTRACE
  ShenanigansException(const std::string& message, const std::source_location& source, std::stacktrace&& stacktrace = std::stacktrace::current()) :
    mMessage(fmt::format("{} [{}:{}]", message, std::filesystem::relative(std::filesystem::path(source.file_name()), GetProjectPath("src")).string(), source.line())),
    mStacktrace(std::move(stacktrace))
  {
  }
#  else
  ShenanigansException(const std::string& message, const std::source_location& source) :
    mMessage(fmt::format("{} [{}:{}]", message, std::filesystem::relative(std::filesystem::path(source.file_name()), GetProjectPath("src")).string(), source.line()))
  {
  }
#  endif
#else
  ShenanigansException(const std::string& message, const std::source_location& source) : mMessage(message) {}
#endif

  const char* what() const noexcept override { return mMessage.c_str(); }

  std::string Log() const
  {
    std::string ret = mMessage;
#ifdef DEVELOP
#  ifdef HAS_STACKTRACE
    ret.append("Exception stack trace:");
    size_t index = 0;
    for (const auto& entry : mStacktrace)
      ret.append(fmt::format("[{}] {} at {}:{}", index++, entry.description(), entry.source_file(), entry.source_line()));
#  endif
#endif
    return ret;
  }
};
