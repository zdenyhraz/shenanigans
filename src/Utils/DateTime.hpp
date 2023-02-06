#pragma once

inline std::string GetCurrentDate()
{
  return fmt::format("{:%Y-%b-%d}", fmt::localtime(std::time(nullptr)));
}

inline std::string GetCurrentTime()
{
  return fmt::format("{:%H:%M:%S}", fmt::localtime(std::time(nullptr)));
}
