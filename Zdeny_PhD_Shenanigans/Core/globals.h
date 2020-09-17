#pragma once
#include "stdafx.h"
#include "IPC/IPC.h"
#include "IPC/IterativePhaseCorrelation.h"

class Globals
{
public:
  Globals()
  {
    IPCset = std::make_unique<IPCsettings>(64, 64, 1, 200);
    IPC = std::make_unique<IterativePhaseCorrelation>(64, 64, 1, 200);
  }

  std::unique_ptr<IPCsettings> IPCset;
  std::unique_ptr<IterativePhaseCorrelation> IPC;
};
