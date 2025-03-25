#pragma once

class Random
{
public:
  template <typename T = double>
  static T Rand(double min = 0., double max = 1.)
  {
    static_assert(std::is_floating_point_v<T> or std::is_integral_v<T>);

    if constexpr (std::is_floating_point_v<T>)
      return std::uniform_real_distribution<T>(min, max)(Get().mGenerator);
    if constexpr (std::is_integral_v<T>)
      return std::uniform_int_distribution<T>(min, max)(Get().mGenerator);
  }

  template <typename T = double>
  static T Randn(double mean = 0., double stddev = 1.)
  {
    static_assert(std::is_floating_point_v<T>);
    return std::normal_distribution<T>(mean, stddev)(Get().mGenerator);
  }

private:
  std::mt19937 mGenerator;

  Random() { mGenerator.seed(std::random_device{}()); }

  static Random& Get()
  {
    static Random rand;
    return rand;
  }
};
