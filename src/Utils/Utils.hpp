#pragma once

template <typename T = f64>
inline auto Zerovect(i32 N, T value = 0.)
{
  return std::vector<T>(N, value);
}

template <typename T = f64>
inline auto Zerovect2(i32 N, i32 M, T value = 0.)
{
  return std::vector<std::vector<T>>(N, Zerovect(M, value));
}

template <typename T>
inline std::vector<T> operator+(const std::vector<T>& vec1, const std::vector<T>& vec2)
{
  std::vector<T> result(vec1.size());
  for (usize i = 0; i < vec1.size(); i++)
    result[i] = vec1[i] + vec2[i];
  return result;
}

template <typename T>
inline std::vector<T> operator-(const std::vector<T>& vec1, const std::vector<T>& vec2)
{
  std::vector<T> result(vec1.size());
  for (usize i = 0; i < vec1.size(); i++)
    result[i] = vec1[i] - vec2[i];
  return result;
}

template <typename T>
inline std::vector<T>& operator+=(std::vector<T>& vec1, const std::vector<T>& vec2)
{
  for (usize i = 0; i < vec1.size(); i++)
    vec1[i] += vec2[i];
  return vec1;
}

template <typename T>
inline std::vector<T>& operator-=(std::vector<T>& vec1, const std::vector<T>& vec2)
{
  for (usize i = 0; i < vec1.size(); i++)
    vec1[i] -= vec2[i];
  return vec1;
}

template <typename T>
inline std::vector<T> operator*(f64 val, const std::vector<T>& vec)
{
  std::vector<T> result(vec.size());
  for (usize i = 0; i < vec.size(); i++)
    result[i] = val * vec[i];
  return result;
}

inline std::string GetCurrentDateTime()
{
  time_t now = time(0);
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  strftime(buf, sizeof(buf), "%Y-%b%d-%H.%M.%S", &tstruct);
  return buf;
}

inline std::string GetCurrentTime()
{
  auto now = time(nullptr);
  char buf[sizeof("12:34:56")];
  strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now));
  return buf;
}

template <typename T>
inline std::vector<T> Iota(T first, usize size)
{
  std::vector<T> vec(size);
  std::iota(vec.begin(), vec.end(), first);
  return vec;
}
