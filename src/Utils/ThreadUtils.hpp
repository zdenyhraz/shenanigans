#pragma once

template <typename T>
inline void LaunchAsync(T fun)
try
{
  std::thread(fun).detach();
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}
catch (...)
{
  LOG_UNKNOWN_EXCEPTION;
}
