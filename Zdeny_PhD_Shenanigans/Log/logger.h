#pragma once
#define LOG_TRACE(...) Logger::Get()->trace(__VA_ARGS__)
#define LOG_DEBUG(...) Logger::Get()->debug(__VA_ARGS__)
#define LOG_SUCC(...) Logger::Get()->info(__VA_ARGS__)
#define LOG_INFO(...) Logger::Get()->warn(__VA_ARGS__)
#define LOG_ERROR(...) Logger::Get()->error(__VA_ARGS__)
#define LOG_FATAL(...) Logger::Get()->critical(__VA_ARGS__)

#define LOG_NEWLINE Logger::Get()->debug("")

#define LOG_DEBUG_IF(c, ...)                                                                                                                                                                           \
  if (c)                                                                                                                                                                                               \
  Logger::Get()->debug(__VA_ARGS__)
#define LOG_SUCC_IF(c, ...)                                                                                                                                                                            \
  if (c)                                                                                                                                                                                               \
  Logger::Get()->info(__VA_ARGS__)
#define LOG_INFO_IF(c, ...)                                                                                                                                                                            \
  if (c)                                                                                                                                                                                               \
  Logger::Get()->warn(__VA_ARGS__)
#define LOG_ERROR_IF(c, ...)                                                                                                                                                                           \
  if (c)                                                                                                                                                                                               \
  Logger::Get()->error(__VA_ARGS__)
#define LOG_FATAL_IF(c, ...)                                                                                                                                                                           \
  if (c)                                                                                                                                                                                               \
  Logger::Get()->critical(__VA_ARGS__)

#define LOG_IFELSE(c, a, b)                                                                                                                                                                            \
  if (c)                                                                                                                                                                                               \
  Logger::Get()->info(c, a) else Logger::Get()->error(c, b)

#define LOG_STARTEND(s, e) std::unique_ptr<LOG_STARTEND_IMPL> log_startend_impl = std::make_unique<LOG_STARTEND_IMPL>(s, e)
#define LOG_FUNCTION(fun) std::unique_ptr<LOG_FUNCTION_IMPL> log_function_impl = std::make_unique<LOG_FUNCTION_IMPL>(fun)

class Logger
{
public:
  Logger()
  {
    spdlogger = spdlog::stdout_color_mt("console");
    spdlogger->set_pattern("[%T] %^%v%$");
    spdlogger->set_level(spdlog::level::trace);
  }

  inline static std::shared_ptr<spdlog::logger>& Get() { return spdlogger; }
  inline static void Init() { logger = std::make_unique<Logger>(); }

private:
  static std::shared_ptr<spdlog::logger> spdlogger;
  static std::unique_ptr<Logger> logger;
};

class LOG_STARTEND_IMPL
{
public:
  LOG_STARTEND_IMPL(const std::string& startMsg, const std::string& endMsg) : EndMsg(endMsg) { LOG_DEBUG(startMsg); }
  ~LOG_STARTEND_IMPL() { LOG_DEBUG(EndMsg); }

private:
  std::string EndMsg;
};

class LOG_FUNCTION_IMPL
{
  using clock = std::chrono::high_resolution_clock;
  using time_point = std::chrono::time_point<clock>;
  using duration = std::chrono::milliseconds;

public:
  LOG_FUNCTION_IMPL(const std::string& funName) : mFunName(funName), mStartTime(clock::now()) { LOG_DEBUG(fmt::format("{} started", mFunName)); }
  ~LOG_FUNCTION_IMPL() { LOG_DEBUG(fmt::format("{} finished ({} ms)", mFunName, TimeSinceEpoch(clock::now()) - TimeSinceEpoch(mStartTime))); }

private:
  static constexpr long long TimeSinceEpoch(const time_point& tmp) { return std::chrono::time_point_cast<duration>(tmp).time_since_epoch().count(); }

  std::string mFunName;
  time_point mStartTime;
};
