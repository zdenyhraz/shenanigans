#pragma once

inline constexpr f64 ToRadians(f64 degrees)
{
  return degrees / 360. * 2 * std::numbers::pi;
}

inline constexpr f64 ToDegrees(f64 radians)
{
  return radians * 360. / (2 * std::numbers::pi);
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

inline f64 Gaussian(f64 x, f64 amp, f64 mid, f64 sigma)
{
  return amp * std::exp(-0.5 * std::pow((x - mid) / sigma, 2));
}

template <typename T>
inline f64 GetQuantile(const std::vector<T>& vec, f64 quan)
{
  std::vector<T> out = vec;
  std::sort(out.begin(), out.end());
  return out[(usize)(quan * (out.size() - 1))];
}
