#pragma once

template <int N>
class FrameAverager
{
  usize index = 0;
  f32 sum = 0;
  std::array<f32, N> data{};

public:
  void Register(f32 value)
  {
    const auto i = index % N;
    sum -= data[i];
    sum += value;
    data[i] = value;
    ++index;
  }

  f32 Get() const { return sum / N; }
};
