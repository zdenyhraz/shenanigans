#pragma once
#include "SystemStatus.hpp"
#include "ScanManager.hpp"

class Instrument
{
public:
  SystemStatus& GetSystemStatus() { return mSystemStatus; }
  ScanManager& GetScanManager() { return mScanManager; }

private:
  SystemStatus mSystemStatus;
  ScanManager mScanManager;
};
