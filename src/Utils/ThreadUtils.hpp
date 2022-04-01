#pragma once

template <typename T>
inline void LaunchAsync(T fun)
{
  std::thread(fun).detach();
}
