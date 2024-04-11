#pragma once

template <int N>
class FPSCounter
{
  usize index = 0;
  std::array<f32, N> times{};

public:
  void RegisterDuration(f32 duration) { times[index++ % N] = duration; }

  f32 GetFPS() const { return 1. / (std::accumulate(times.begin(), times.end(), 0.) / N); }
};
