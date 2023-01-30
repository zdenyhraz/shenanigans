#pragma once

template <typename T>
inline void LaunchAsync(T&& fun)
{
  std::thread(
      [=]()
      {
        try
        {
          fun();
        }
        catch (const std::exception& e)
        {
          LOG_EXCEPTION(e);
        }
        catch (...)
        {
          LOG_UNKNOWN_EXCEPTION;
        }
      })
      .detach();
}
