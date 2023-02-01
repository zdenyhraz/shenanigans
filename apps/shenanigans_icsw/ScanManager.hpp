#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include <fmt/format.h>
#include "Utils.hpp"

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

  void ScanLoop() { Log("Scanning"); }
};
