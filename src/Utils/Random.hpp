#pragma once

class Random
{
public:
  static std::mt19937::result_type Rand() { return Get().mGenerator(); }

  template <typename T = f64>
  static T Randu(T min = 0., T max = 1.)
  {
    static_assert(std::is_floating_point_v<T> or std::is_integral_v<T>);

    if constexpr (std::is_floating_point_v<T>)
      return std::uniform_real_distribution<T>{min, max}(Get().mGenerator);
    if constexpr (std::is_integral_v<T>)
      return std::uniform_int_distribution<T>{min, max}(Get().mGenerator);
  }

  template <typename T = f64>
  static T Randn(T mean = 0., T stddev = 1.)
  {
    static_assert(std::is_floating_point_v<T>);
    return std::normal_distribution<T>{mean, stddev}(Get().mGenerator);
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
