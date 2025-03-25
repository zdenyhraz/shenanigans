#pragma once

template <int N>
class FrameAverager
{
  size_t index = 0;
  float sum = 0;
  std::array<float, N> data{};

public:
  void Register(float value)
  {
    const auto i = index % N;
    sum -= data[i];
    sum += value;
    data[i] = value;
    ++index;
  }

  float Get() const { return sum / N; }
};
