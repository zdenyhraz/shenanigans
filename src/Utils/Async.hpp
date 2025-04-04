#pragma once

template <typename T>
inline void LaunchAsync(T&& fun)
{
  std::thread(
      [fun]()
      {
        try
        {
          fun();
        }
        catch (const ShenanigansException& e)
        {
          e.Log();
        }
        catch (const std::exception& e)
        {
          LOG_EXCEPTION(e);
        }
      })
      .detach();
}
