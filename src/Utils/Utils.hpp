#pragma once

inline f64 RandU()
{
  return static_cast<f64>(rand()) / RAND_MAX;
}

inline f64 RandRange(f64 min_, f64 max_)
{
  return min_ + RandU() * (max_ - min_);
}

inline f64 ClampSmooth(f64 x_new, f64 x_prev, f64 clampMin, f64 clampMax)
{
  return x_new < clampMin ? (x_prev + clampMin) / 2 : x_new > clampMax ? (x_prev + clampMax) / 2 : x_new;
}

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
inline constexpr T Sqr(T x)
{
  return x * x;
}

template <typename T>
inline f64 Mean(const std::vector<T>& vec)
{
  return std::accumulate(vec.begin(), vec.end(), 0.) / vec.size();
}

template <typename T>
inline f64 Stddev(const std::vector<T>& vec)
{
  const f64 m = Mean(vec);
  f64 stdev = 0;
  for (const auto x : vec)
    stdev += Sqr(x - m);
  stdev /= vec.size();
  return std::sqrt(stdev);
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

template <typename T>
inline f64 Median(std::vector<T>& vec)
{
  if (vec.size() % 2 == 0)
  {
    // std::partial_sort(vec.begin(), vec.begin() + vec.size() / 2, vec.end());
    std::sort(vec.begin(), vec.end());
    return 0.5 * vec[vec.size() / 2] + 0.5 * vec[vec.size() / 2 - 1];
  }

  std::nth_element(vec.begin(), vec.begin() + vec.size() / 2, vec.end());
  return vec[vec.size() / 2];
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
  time_t now = time(0);
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  strftime(buf, sizeof(buf), "%H:%M:%S", &tstruct);
  return buf;
}

inline f64 Gaussian(f64 x, f64 amp, f64 mid, f64 sigma)
{
  return amp * std::exp(-0.5 * std::pow((x - mid) / sigma, 2));
}

template <typename T>
inline std::vector<T> Iota(T first, usize size)
{
  std::vector<T> vec(size);
  std::iota(vec.begin(), vec.end(), first);
  return vec;
}

inline f64 ToDegrees(f64 rad)
{
  return rad * Constants::Rad;
}

inline f64 ToRadians(f64 deg)
{
  return deg / Constants::Rad;
}

template <typename T>
inline f64 GetQuantile(const std::vector<T>& vec, f64 quan)
{
  std::vector<T> out = vec;
  std::sort(out.begin(), out.end());
  return out[(usize)(quan * (out.size() - 1))];
}
