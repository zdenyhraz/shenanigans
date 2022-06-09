#pragma once
#include "SystemStatus.hpp"
#include "ScanManager.hpp"

class Instrument
{
public:
  Instrument() { fmt::print("Instrument()\n"); }

  ~Instrument() { fmt::print("~Instrument()\n"); }

  void Start()
  {
    mThread.Start([this]() { MainLoop(); });
  }

  void Stop() { mThread.Stop(); }

private:
  ThreadLoop mThread{"Main loop"};
  SystemStatus mSystemStatus;
  ScanManager mScanManager;

  void MainLoop()
  {
    const int message = std::rand() % 10; // simulate random incomming messages
    DispatchMessage(message);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  void DispatchMessage(int message)
  {
    switch (message)
    {
    case 0:
      fmt::print("> Message received: Start system status monitoring {}\n", GetCurrentThreadId());
      mSystemStatus.StartMonitoring();
      break;
    case 1:
      fmt::print("> Message received: Stop system status monitoring {}\n", GetCurrentThreadId());
      mSystemStatus.StopMonitoring();
      break;
    case 2:
      fmt::print("> Message received: Start scanning {}\n", GetCurrentThreadId());
      mScanManager.StartScanning();
      break;
    case 3:
      fmt::print("> Message received: Stop scanning {}\n", GetCurrentThreadId());
      mScanManager.StopScanning();
      break;
    default:
      break;
    }
  }
};
