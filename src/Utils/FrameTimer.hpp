#pragma once
#include "FrameAverager.hpp"

template <typename N>
class FrameTimer
{
  FrameAverage<N> averager;
  std::chrono::time_point start;

public:
  void Start() { start = std::chrono::high_resolution_clock::now(); }

  void Stop()
  {
    const auto end = std::chrono::high_resolution_clock::now();
    averager.Register(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
  }

  f32 GetDurationMilliseconds() { averager.Get(); }
};
