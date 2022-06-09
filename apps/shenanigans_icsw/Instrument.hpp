#pragma once
#include "SystemStatus.hpp"
#include "ScanManager.hpp"
#include "CommInterface.hpp"

class Instrument
{
public:
  Instrument() { mCommInterface.StartListening(); }
  ~Instrument() { mCommInterface.StopListening(); }

private:
  SystemStatus mSystemStatus;
  ScanManager mScanManager;
  CommInterface mCommInterface{mSystemStatus, mScanManager};
};
