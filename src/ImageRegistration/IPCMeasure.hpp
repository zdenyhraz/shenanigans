#pragma once

class IPC;

class IPCMeasure
{
  static constexpr f64 mQuanT = 0.95;

public:
  static void MeasureAccuracy(const IPC& ipc, const std::string& path, i32 iters, f64 maxShift, f64 noiseStddev, f32* progress = nullptr);
};
