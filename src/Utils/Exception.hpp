#pragma once

#ifdef __cpp_lib_stacktrace

class ShenanigansException : public std::exception
{
  std::string mMessage;
  std::stacktrace mStacktrace;

public:
  ShenanigansException(const std::string& message, const std::source_location& source, std::stacktrace&& stacktrace) :
    mMessage(fmt::format("{} [{}:{}]", message, std::filesystem::relative(std::filesystem::path(source.file_name()), GetProjectDirectoryPath("src")).string(), source.line())),
    mStacktrace(std::move(stacktrace))
  {
  }

  const char* what() const noexcept override { return mMessage.c_str(); }

  void Log() const
  {
    LOG_ERROR(mMessage);
    LOG_WARNING("Exception stack trace:\n{}", std::to_string(mStacktrace));
  }
};
#else
class ShenanigansException : public std::exception
{
  std::string mMessage;

public:
  ShenanigansException(const std::string& message, const std::source_location& source) :
    mMessage(fmt::format("{} [{}:{}]", message, std::filesystem::relative(std::filesystem::path(source.file_name()), GetProjectDirectoryPath("src")).string(), source.line()))
  {
  }

  const char* what() const noexcept override { return mMessage.c_str(); }

  void Log() const { LOG_ERROR(mMessage); }
};
#endif
