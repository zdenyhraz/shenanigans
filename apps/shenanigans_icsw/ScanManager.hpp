#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include "ThreadUtils.hpp"

class ScanManager
{
public:
  void StartScanning()
  {
    mThread.Start([this]() { ScanLoop(); });
  }

  void StopScanning() { mThread.Stop(); }

private:
  ThreadLoop mThread{"Scan loop"};

  void ScanLoop()
  {
    fmt::print("Scanning... {}\n", GetCurrentThreadId());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
};
