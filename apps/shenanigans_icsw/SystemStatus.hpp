#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include <fmt/format.h>
#include "Utils.hpp"

class SystemStatus
{
public:
  void StartMonitoring()
  {
    mThread.Start([this]() { MonitoringLoop(); });
  }

  void StopMonitoring() { mThread.Stop(); }

  double GetHighVacuumPressure() const { return mHighVacuumPressure; }
  double GetCIDGasPressure() const { return mCIDGasPressure; }
  double GetTotalIonCurrent() const { return mTotalIonCurrent; }

private:
  ThreadLoop mThread{"Monitoring loop"};

  double mHighVacuumPressure = 0;
  double mCIDGasPressure = 0;
  double mTotalIonCurrent = 0;

  static void MonitoringLoop() { Log("Monitoring"); }
};
